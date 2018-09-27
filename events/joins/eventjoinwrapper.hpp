#pragma once

#include "eventjoinwrapper.h"
#include "handlereventjoin.h"
#include "../handlers/handlercast.hpp"


namespace events {

namespace joins {


template<class TSome, class ...TParams>
EventJoinWrapper::EventJoinWrapper( IEvent<TParams...>& _event, TSome&& handler ) :
    m_eventJoin( std::make_shared<HandlerEventJoin<TParams...>>( _event, ::events::handlers::HandlerCast<TSome>::cast<TParams...>( handler ) ) )
{
}


} // joins

} // events
