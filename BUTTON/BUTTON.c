/**
 * @file button.c
 * @brief Driver for managing GPIO buttons.
 * 
 * Provides functions for button initialization, handling button presses, 
 * and callback execution for various press types (normal, long, double-click, etc.).
 * 
 * Functions:
 * - BUTTON_Init: Initializes a button with GPIO, active state, mode, and handler.
 * - BUTTON_Deinit: Deinitializes a button.
 * - BUTTON_Update: Updates button states and handles press actions.
 * - Set_DebounceTime, SetTime_Hold_mode, SetTime_Toggle_mode: Adjust button timing.
 * 
 * Usage:
 * - Call BUTTON_Update regularly to process button events.
 * - Define a handler function to manage button presses.
 */




#include "button.h"

// Button pool array to store button states
static Button_t buttonPool[BUTTON_MAX];
// Array of pointers to initialized buttons
static Button_t* buttons[BUTTON_MAX];
// Count of initialized buttons
static uint8_t buttonCount = 0;

// Function to read the current state of the button
static inline uint8_t BUTTON_Read(Button_t* btn) {
    return ((btn->GPIOx->IDR & btn->GPIO_Pin) ? 1 : 0) == btn->ActiveState; 	
} // ActiveState = 0 is PULLUP -- 1  is PULLDOWN


// Configure the button in Toggle mode
static void ConfigureToggleMode(Button_t* btn) {
    btn->Mode = BUTTON_Mode_Toggle;
    btn->NormalTime = 300;
    btn->LongTime = 1000;
    btn->VeryLongTime = 3000;
    btn->DoubleClickTime = 0;
    btn->FirstClickDone = 0;
}

// Configure the button in Hold mode
static void ConfigureHoldMode(Button_t* btn) {
    btn->Mode = BUTTON_Mode_Hold;
    btn->RepeatDelay = 500;
    btn->RepeatInterval = 200;
    btn->RepeatStarted = 0;
}

// Initialize a button with specific GPIO, mode, and handler
Button_t* BUTTON_Init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t ActiveState,
                      ButtonMode_t mode, void (*Handler)(Button_t*, ButtonPressType_t)) {
    if (buttonCount >= BUTTON_MAX) return NULL;

    Button_t* btn = &buttonPool[buttonCount];
    buttons[buttonCount++] = btn;

    btn->GPIOx = GPIOx;
    btn->GPIO_Pin = GPIO_Pin;
    btn->ActiveState = ActiveState;

    btn->LastStatus = 0;
    btn->State = BUTTON_STATE_START;
    btn->DebounceTime = 50;
    btn->Handler = (Handler != NULL) ? Handler : BUTTON_Callback;

    // Configure button based on selected mode
    if (mode == BUTTON_Mode_Toggle) {
        ConfigureToggleMode(btn);
    } else if (mode == BUTTON_Mode_Hold) {
        ConfigureHoldMode(btn);
    } else {
        buttonCount--; // Undo if mode is invalid
        return NULL;
    }

    return btn;
}

// Deinitialize a button and remove it from the pool
void BUTTON_Deinit(Button_t* btn) {
    if (!btn) return;

    for (uint8_t i = 0; i < buttonCount; ++i) {
        if (buttons[i] == btn) {
            for (uint8_t j = i; j < buttonCount - 1; ++j) {
                buttons[j] = buttons[j + 1];
                buttonPool[j] = buttonPool[j + 1];
            }
            buttonCount--;
            break;
        }
    }
}

// Set the debounce time for a button
void Set_DebounceTime(Button_t* btn, uint8_t debounceTime) {
    btn->DebounceTime = debounceTime;
}

// Set the repeat delay and interval for Hold mode
void SetTime_Hold_mode(Button_t* btn, uint16_t delay, uint16_t interval) {
    if (!btn) return;
    btn->RepeatDelay = delay;
    btn->RepeatInterval = interval;
}

// Set times for press durations in Toggle mode
void SetTime_Toggle_mode(Button_t* btn, uint16_t time4Double, uint16_t normal, uint16_t longer, uint16_t very_long) {
    if (!btn) return;
    btn->DoubleClickTime = time4Double;
    btn->NormalTime = normal;
    btn->LongTime = longer;
    btn->VeryLongTime = very_long;
}

// Update the state of all buttons
void BUTTON_Update(void) {
    uint32_t now = HAL_GetTick(); // Get the current time in milliseconds

    for (uint8_t i = 0; i < buttonCount; ++i) {
        Button_t* btn = buttons[i];  // Access the current button
        uint8_t currentStatus = BUTTON_Read(btn); // Read the current status of the button (pressed or not)

        switch (btn->State) {
            case BUTTON_STATE_START:
                if (currentStatus) {
                    // Button is pressed, record start time and switch to debounce state
                    btn->StartTime = now;
                    btn->State = BUTTON_STATE_DEBOUNCE; // Enter debounce phase to filter out noise
                }
                break;

            case BUTTON_STATE_DEBOUNCE:
                // If debounce time has passed, transition to pressed state if button is still pressed
                if (now - btn->StartTime >= btn->DebounceTime) {
                    if (currentStatus) {
                        btn->State = BUTTON_STATE_PRESSED; // Transition to pressed state
                        btn->LastRepeatTime = now; // Record the last time the button was pressed

                        // Handle button press in Hold mode if applicable
                        if (btn->Mode == BUTTON_Mode_Hold && btn->Handler) {
                            btn->Handler(btn, BUTTON_PressType_RepeatOnce); // Trigger repeat once event
                        }
                    } else {
                        // If button is released, return to the start state
                        btn->State = BUTTON_STATE_START;
                    }
                }
                break;

            case BUTTON_STATE_PRESSED:
                // Button is pressed, now checking for release or repeating actions
                if (!currentStatus) {
                    uint32_t pressDuration = now - btn->StartTime; // Calculate how long the button was pressed

                    // Handle Toggle mode if button is released
                    if (btn->Mode == BUTTON_Mode_Toggle && btn->Handler) {
                        if (btn->FirstClickDone) { // Check if first click is completed
                            if (now - btn->FirstClickReleaseTime <= btn->DoubleClickTime) {
                                // If within double-click time, trigger double-click event
                                btn->Handler(btn, BUTTON_PressType_Double);
                                btn->FirstClickDone = 0; // Reset first click flag
                            } else {
                                // Determine press type based on press duration
                                ButtonPressType_t type =
                                    (btn->LastPressDuration >= btn->VeryLongTime) ? BUTTON_PressType_VeryLong :
                                    (btn->LastPressDuration >= btn->LongTime) ? BUTTON_PressType_Long :
                                    (btn->LastPressDuration >= btn->NormalTime) ? BUTTON_PressType_Normal :
                                    BUTTON_PressType_OnPressed;

                                // Trigger the appropriate handler based on press type
                                btn->Handler(btn, type);
                                btn->FirstClickDone = 1; // Mark first click as done
                                btn->FirstClickReleaseTime = now; // Store release time for future comparison
                                btn->LastPressDuration = pressDuration; // Store press duration
                            }
                        } else {
                            // First click handling if it hasn't been processed yet
                            btn->FirstClickDone = 1;
                            btn->FirstClickReleaseTime = now;
                            btn->LastPressDuration = pressDuration;
                        }
                    }

                    // After processing the button release, reset to start state
                    btn->State = BUTTON_STATE_START;
                    btn->RepeatStarted = 0; // Reset repeat action flag
                } else if (btn->Mode == BUTTON_Mode_Hold) {
                    // Handle button held down for repeat functionality
                    if (!btn->RepeatStarted && now - btn->StartTime >= btn->RepeatDelay) {
                        btn->RepeatStarted = 1; // Start repeat action
                        btn->LastRepeatTime = now; // Record time of repeat
                        if (btn->Handler) btn->Handler(btn, BUTTON_PressType_Repeat); // Trigger repeat event
                    } else if (btn->RepeatStarted && now - btn->LastRepeatTime >= btn->RepeatInterval) {
                        // Continue repeating if interval time has passed
                        btn->LastRepeatTime = now;
                        if (btn->Handler) btn->Handler(btn, BUTTON_PressType_Repeat); // Trigger repeat event
                    }
                }
                break;

            default:
                // Handle any unexpected state transitions
                btn->State = BUTTON_STATE_START;
                break;
        }

        // Handle Toggle mode for release timing, checking if double click time has passed
        if (btn->Mode == BUTTON_Mode_Toggle && btn->FirstClickDone) {
            if (now - btn->FirstClickReleaseTime > btn->DoubleClickTime) {
                btn->FirstClickDone = 0; // Reset first click after double-click time

                // Determine press type based on duration of the press
                ButtonPressType_t type =
                    (btn->LastPressDuration >= btn->VeryLongTime) ? BUTTON_PressType_VeryLong :
                    (btn->LastPressDuration >= btn->LongTime) ? BUTTON_PressType_Long :
                    (btn->LastPressDuration >= btn->NormalTime) ? BUTTON_PressType_Normal :
                    BUTTON_PressType_OnPressed;

                // Trigger the appropriate handler for the press type
                if (btn->Handler) btn->Handler(btn, type);
            }
        }
    }
}
 

// Default callback for button presses
__weak void BUTTON_Callback(Button_t* btn, ButtonPressType_t type) {
    // Default implementation, can be overridden by user
}


/*
We have two types of button modes in this system:

1. Toggle Mode:
	 In Toggle mode, the button alternates between two states each time it is pressed.
	 This can be useful for actions that toggle on/off or switch between two options.
	 In this mode, we can define different behaviors for different press durations,
	 such as a normal press, long press, very long press, and double-click. 
	 Each of these press types triggers a different action
2. Hold Mode:
	 Hold mode is used when the button needs to be held down to perform a continuous action, 
	 such as repeating an action at a certain interval. When a button is pressed and held, 
	 it can trigger an action once and then continue triggering the action at a fixed interval until
	 the button is released. This is commonly used for actions that require repeated input, 
	 like increasing the volume or scrolling.

*/ 

/* 

=================================== How to USE ==========================================

void Toggle_ButtonHandler(Button_t* btn, ButtonPressType_t type) {
	switch (type) {
		case BUTTON_PressType_Double: // Triggered on double-click, handle double-click action here.
			break;
		case BUTTON_PressType_OnPressed: // Triggered immediately when button is pressed down, for instant response.
			break;
		case BUTTON_PressType_Normal: // Triggered on a quick press and release, useful for single-click actions.
			break;
		case BUTTON_PressType_Long: // Triggered when button is held for a long time, good for special actions.
			break;
		case BUTTON_PressType_VeryLong: // Triggered on very long hold, suitable for critical actions like reset or mode switch.
			break;
		default: // Unrecognized press type, do nothing.
			break;
	}
}

void Hold_ButtonHandler(Button_t* btn, ButtonPressType_t type) {
    if (type == BUTTON_PressType_RepeatOnce) {
        // Handle first time the button is held down
    } else if (type == BUTTON_PressType_Repeat) {
        // Handle repeated hold events while button remains pressed
    }
}


================================= Example for MODE TOGGLE =========================================


# 1. Create function toggles the LED state (GPIOC PIN 13) when the button is pressed in Toggle mode.

void Toggle_ButtonHandler1(Button_t* btn, ButtonPressType_t type) {
	switch (type) {
		default:
				HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
				break;
	}
}


# 2. Declare in main()

	Button_t* btn2 = BUTTON_Init(GPIOA, GPIO_PIN_1, 0, BUTTON_Mode_Toggle, Toggle_ButtonHandler1);
	SetTime_Toggle_mode(btn2, 100, 1000, 2000, 5000);
				//100: Max gap between 2 presses for double click - if dont use double click set it = 0
				***noted: if use doubleclick button will delay = gap between 2 presses time.
				//1000: Press =1000ms = normal press
				//1500: Press =1500ms = long press
				//2000: Press =2000ms = very long press
	Set_DebounceTime(btn2, 50);					// set Debounce time for btn1 10-50(ms)
	

# 3. Update button every loop

while (1)
{
	BUTTON_Update();
}




================================= Example for MODE HOLD =========================================


# 1. Createfunction toggles the LED state (GPIOC PIN 13) when the button is pressed in Hold mode.

void Hold_ButtonHandler1(Button_t* btn, ButtonPressType_t type) {
	if(type == BUTTON_PressType_RepeatOnce) HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
//	else HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}



# 2. Declare in main()

Button_t* btn1 = BUTTON_Init(GPIOA, GPIO_PIN_0, 0, BUTTON_Mode_Hold, Hold_ButtonHandler1);
	SetTime_Hold_mode(btn1, 1000, 200); 	//	1000 is time to enable REPEAT,  200 time every REPEAT
	Set_DebounceTime(btn1, 30);					// set Debounce time for btn1 10-50(ms)
	


# 3. Update button every loop

while (1)
{
	BUTTON_Update();
}



*/

