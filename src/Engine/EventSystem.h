#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <memory>
#include <typeindex>
#include <string>
#include <algorithm>

namespace Engine {

/**
 * @brief Base class for all events
 *
 * Events are used to communicate between different parts of the engine
 * without tight coupling. Each event should contain all necessary data.
 */
class Event {
public:
    virtual ~Event() = default;

    /**
     * @brief Check if the event has been handled
     * @return True if handled, false otherwise
     */
    bool IsHandled() const { return m_Handled; }

    /**
     * @brief Mark the event as handled
     */
    void SetHandled(bool handled = true) { m_Handled = handled; }

    /**
     * @brief Get the event type name
     * @return Type name as string
     */
    virtual std::string GetTypeName() const = 0;

protected:
    bool m_Handled = false;
};

/**
 * @brief Event listener callback type
 */
template<typename T>
using EventCallback = std::function<void(const T&)>;

/**
 * @brief Event listener handle for unsubscribing
 */
using EventListenerID = uint64_t;

/**
 * @brief Event system for publishing and subscribing to game events
 *
 * Implements the observer pattern with type-safe event handling.
 * Events are dispatched immediately when published.
 */
class EventSystem {
public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the event system instance
     */
    static EventSystem& GetInstance();

    /**
     * @brief Subscribe to an event type
     * @tparam T Event type (must derive from Event)
     * @param callback Function to call when event is published
     * @return Listener ID for unsubscribing
     */
    template<typename T>
    EventListenerID Subscribe(EventCallback<T> callback);

    /**
     * @brief Unsubscribe from an event type
     * @tparam T Event type
     * @param listenerID Listener ID returned from Subscribe
     */
    template<typename T>
    void Unsubscribe(EventListenerID listenerID);

    /**
     * @brief Publish an event immediately
     * @tparam T Event type
     * @param event Event instance to publish
     */
    template<typename T>
    void Publish(const T& event);

    /**
     * @brief Clear all event listeners
     */
    void Clear();

    /**
     * @brief Get the number of listeners for an event type
     * @tparam T Event type
     * @return Number of active listeners
     */
    template<typename T>
    size_t GetListenerCount() const;

private:
    EventSystem() = default;
    ~EventSystem() = default;

    // Prevent copying
    EventSystem(const EventSystem&) = delete;
    EventSystem& operator=(const EventSystem&) = delete;

    struct BaseEventListener {
        virtual ~BaseEventListener() = default;
        EventListenerID id;
    };

    template<typename T>
    struct EventListener : public BaseEventListener {
        EventCallback<T> callback;
    };

    using EventListenerPtr = std::shared_ptr<BaseEventListener>;
    using EventListenerList = std::vector<EventListenerPtr>;

    std::unordered_map<std::type_index, EventListenerList> m_Listeners;
    EventListenerID m_NextListenerID = 1;
};

// Template implementations

template<typename T>
EventListenerID EventSystem::Subscribe(EventCallback<T> callback) {
    static_assert(std::is_base_of<Event, T>::value,
                  "T must derive from Event");

    auto listener = std::make_shared<EventListener<T>>();
    listener->id = m_NextListenerID++;
    listener->callback = std::move(callback);

    std::type_index typeIndex = std::type_index(typeid(T));
    m_Listeners[typeIndex].push_back(listener);

    return listener->id;
}

template<typename T>
void EventSystem::Unsubscribe(EventListenerID listenerID) {
    static_assert(std::is_base_of<Event, T>::value,
                  "T must derive from Event");

    std::type_index typeIndex = std::type_index(typeid(T));
    auto it = m_Listeners.find(typeIndex);

    if (it != m_Listeners.end()) {
        auto& listeners = it->second;
        listeners.erase(
            std::remove_if(listeners.begin(), listeners.end(),
                [listenerID](const EventListenerPtr& listener) {
                    return listener->id == listenerID;
                }),
            listeners.end()
        );

        // Remove the event type entry if no more listeners
        if (listeners.empty()) {
            m_Listeners.erase(it);
        }
    }
}

template<typename T>
void EventSystem::Publish(const T& event) {
    static_assert(std::is_base_of<Event, T>::value,
                  "T must derive from Event");

    std::type_index typeIndex = std::type_index(typeid(T));
    auto it = m_Listeners.find(typeIndex);

    if (it != m_Listeners.end()) {
        // Create a copy of the listener list to allow modification during iteration
        auto listeners = it->second;

        for (const auto& baseListener : listeners) {
            if (event.IsHandled()) {
                break;
            }

            auto listener = std::static_pointer_cast<EventListener<T>>(baseListener);
            if (listener && listener->callback) {
                listener->callback(event);
            }
        }
    }
}

template<typename T>
size_t EventSystem::GetListenerCount() const {
    static_assert(std::is_base_of<Event, T>::value,
                  "T must derive from Event");

    std::type_index typeIndex = std::type_index(typeid(T));
    auto it = m_Listeners.find(typeIndex);

    return (it != m_Listeners.end()) ? it->second.size() : 0;
}

// Common event types

/**
 * @brief Event fired when a key is pressed
 */
class KeyPressedEvent : public Event {
public:
    KeyPressedEvent(int keyCode, bool repeat = false)
        : m_KeyCode(keyCode), m_Repeat(repeat) {}

    int GetKeyCode() const { return m_KeyCode; }
    bool IsRepeat() const { return m_Repeat; }

    std::string GetTypeName() const override { return "KeyPressedEvent"; }

private:
    int m_KeyCode;
    bool m_Repeat;
};

/**
 * @brief Event fired when a key is released
 */
class KeyReleasedEvent : public Event {
public:
    explicit KeyReleasedEvent(int keyCode) : m_KeyCode(keyCode) {}

    int GetKeyCode() const { return m_KeyCode; }

    std::string GetTypeName() const override { return "KeyReleasedEvent"; }

private:
    int m_KeyCode;
};

/**
 * @brief Event fired when the window is resized
 */
class WindowResizeEvent : public Event {
public:
    WindowResizeEvent(int width, int height)
        : m_Width(width), m_Height(height) {}

    int GetWidth() const { return m_Width; }
    int GetHeight() const { return m_Height; }

    std::string GetTypeName() const override { return "WindowResizeEvent"; }

private:
    int m_Width;
    int m_Height;
};

/**
 * @brief Event fired when the window is closed
 */
class WindowCloseEvent : public Event {
public:
    WindowCloseEvent() = default;

    std::string GetTypeName() const override { return "WindowCloseEvent"; }
};

} // namespace Engine
