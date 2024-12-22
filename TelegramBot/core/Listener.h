#ifndef LISTENER_H
#define LISTENER_H

#include <string>

class Listener {
public:
    virtual void update(const std::string& message) = 0;
    virtual ~Listener() = default;
};

#endif // LISTENER_H
