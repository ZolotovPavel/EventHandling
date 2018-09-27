#pragma once

#include <memory>


namespace events {

namespace handlers {


template<class ...TParams> class AbstractEventHandler;


template<class ...Types>
using TEventHandlerPtr = std::shared_ptr<AbstractEventHandler<Types...>>;


} // handlers

} // events
