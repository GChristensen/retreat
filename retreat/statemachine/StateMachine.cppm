export module StateMachine;

export class StateMachine {
public:
    StateMachine();

    void setIdle();
    bool isIdle() { return mockState; };
    void setAlert();
    void setLocked();

private:
    bool mockState = true;
};


module :private;

StateMachine::StateMachine() {

}

void StateMachine::setIdle() {
    mockState = true;
}

void StateMachine::setAlert() {
    mockState = false;
}

void StateMachine::setLocked() {

}
