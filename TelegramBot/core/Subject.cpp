#include "Subject.h"

void Subject::addListenerInternal(const std::shared_ptr<Listener>& listener) {
    listeners.push_back(listener);
}

void Subject::removeListenerInternal(const std::shared_ptr<Listener>& listener) {
    listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end());
}

void Subject::notifyListenersInternal(const std::string& message) {
    for (const auto& listener : listeners) {
        listener->update(message);
    }
}

void Subject::addListener(const std::shared_ptr<Listener>& listener) {
    addListenerInternal(listener);
}

void Subject::removeListener(const std::shared_ptr<Listener>& listener) {
    removeListenerInternal(listener);
}

void Subject::notifyListeners(const std::string& message) {
    notifyListenersInternal(message);
}
