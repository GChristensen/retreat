export module StateWindowAdapter;

import <memory>;

export class StateWindowAdapter {
public:
    virtual void onTimer() {}
};

export using StateWindowPtr = std::shared_ptr<StateWindowAdapter>;