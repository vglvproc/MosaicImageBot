#ifndef SUBJECT_H
#define SUBJECT_H

#include <vector>
#include <memory>
#include <algorithm>
#include "Listener.h"

class Subject {
private:
    std::vector<std::shared_ptr<Listener>> listeners;

protected:
    virtual void addListenerInternal(const std::shared_ptr<Listener>& listener);
    virtual void removeListenerInternal(const std::shared_ptr<Listener>& listener);
    virtual void notifyListenersInternal(const std::any& message);

public:
    void addListener(const std::shared_ptr<Listener>& listener);
    void removeListener(const std::shared_ptr<Listener>& listener);
    void notifyListeners(const std::any& message);

    virtual ~Subject() = default;
};

#endif // SUBJECT_H
