#include "EventSystem.h"

namespace Engine {

EventSystem& EventSystem::GetInstance() {
    static EventSystem instance;
    return instance;
}

void EventSystem::Clear() {
    m_Listeners.clear();
    m_NextListenerID = 1;
}

} // namespace Engine
