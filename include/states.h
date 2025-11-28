#ifndef APP_STATES_H
#define APP_STATES_H

typedef enum {
    STATE_INIT,
    STATE_MANUAL,
    STATE_SETTINGS,
    STATE_AUTOMATIC,
    NUM_STATES
}State_t;

typedef struct{
    State_t state;
    void (*state_function)(void);
} StateMachine_t;

void fn_INIT(void);
void fn_MANUAL(void);
void fn_AUTOMATIC(void);
void fn_SETTINGS(void);

#endif
