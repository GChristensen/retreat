export module Monitor;

import <memory>;

export class Monitor {
public:
    virtual bool isSatisfied() = 0;
};

export using MonitorPtr = std::shared_ptr<Monitor>;