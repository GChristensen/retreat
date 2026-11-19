module;

#include <memory>

export module StateWindowAdapter;


export class StateWindowAdapter {
public:
    virtual void onTimer() {}
};

export using StateWindowPtr = std::shared_ptr<StateWindowAdapter>;