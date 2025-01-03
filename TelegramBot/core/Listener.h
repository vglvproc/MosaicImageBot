#ifndef LISTENER_H
#define LISTENER_H

#include <string>
#include <any>

class Listener {
public:
    virtual void update(const std::any& message) = 0;
    virtual ~Listener() = default;
};

#endif // LISTENER_H
