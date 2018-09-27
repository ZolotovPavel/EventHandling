#pragma once

#include <type_traits>
#include <list>
#include <memory>
#include <shared_mutex>
#include <algorithm>
#include <assert.h>
#include "handlers/abstracteventhandler.hpp"
#include "handlers/eventhandlerptr.h"
#include "handlers/handlercast.hpp"
#include "joins/eventjoinwrapper.hpp"


namespace events {


namespace joins
{
    template<class ...TParams> class HandlerEventJoin;
}

template<class ...TParams>
class IEvent
{
    public:

        template<class TSome>
        EventJoin operator+=( TSome&& some )
        {
            EventJoin result( *this, std::forward<TSome>( some ) );
            result.join();
            return result;
        }

        template<class TSome>
        bool operator-=( TSome&& some )
        {
            return removeHandler( handlers::HandlerCast<TSome>::template cast<TParams...>( some ) );
        }

    protected:

        using TMyEventHandlerPtr = handlers::TEventHandlerPtr<TParams...>;

        IEvent() {}

        virtual bool isHandlerAdded( const TMyEventHandlerPtr& eventHandler ) const = 0;
        virtual bool addHandler( TMyEventHandlerPtr eventHandler ) = 0;
        virtual bool removeHandler( TMyEventHandlerPtr eventHandler ) = 0;

    friend class joins::HandlerEventJoin<TParams...>;
};


template<class ...TParams>
class TEvent : public IEvent<TParams...>
{
    using TMyEventHandlerPtr = typename IEvent<TParams...>::TMyEventHandlerPtr;
    using TEventHandlerIt = typename std::list<TMyEventHandlerPtr>::const_iterator;

    public:

        TEvent() :
            m_handlers(),
            m_currentIt(),
            m_isCurrentItRemoved( false ),
            m_handlerListMutex()
        {
        }

        void operator()( TParams... params )
        {
            m_handlerListMutex.lock_shared();
            
            m_isCurrentItRemoved = false;
            m_currentIt = m_handlers.begin();
            while( m_currentIt != m_handlers.end() )
            {
                m_handlerListMutex.unlock_shared();
                ( *m_currentIt )->call( params... );
                m_handlerListMutex.lock_shared();

                if( m_isCurrentItRemoved )
                {
                    m_isCurrentItRemoved = false;

                    TEventHandlerIt removedIt = m_currentIt;
                    ++m_currentIt;

                    deleteHandler( removedIt );
                }
                else
                {
                    ++m_currentIt;
                }
            }

            m_handlerListMutex.unlock_shared();
        }

    protected:

        virtual bool isHandlerAdded( const TMyEventHandlerPtr& eventHandler ) const override
        {
            std::shared_lock<std::shared_mutex> _handlerListMutexLock( m_handlerListMutex );

            return ( findEventHandler( eventHandler ) != m_handlers.end() );

        }
        virtual bool addHandler( TMyEventHandlerPtr eventHandler ) override
        {
            std::unique_lock<std::shared_mutex> _handlerListMutexLock( m_handlerListMutex );

            if( findEventHandler( eventHandler ) == m_handlers.end() )
            {
                m_handlers.push_back( std::move( eventHandler ) );
                return true;
            }
            return false;
        }
        virtual bool removeHandler( TMyEventHandlerPtr eventHandler ) override
        {
            std::unique_lock<std::shared_mutex> _handlerListMutexLock( m_handlerListMutex );

            auto it = findEventHandler( eventHandler );
            if( it != m_handlers.end() )
            {
                if( it == m_currentIt )
                    m_isCurrentItRemoved = true;
                else
                    deleteHandler( it );

                return true;
            }
            return false;
        }

    private:

        // использовать под залоченным для чтения 'm_handlerListMutex'
        inline TEventHandlerIt findEventHandler( const TMyEventHandlerPtr& eventHandler ) const noexcept
        {
            return std::find_if( m_handlers.cbegin(), m_handlers.cend(), [ &eventHandler ]( const TMyEventHandlerPtr& oneHandler )
            {
                return ( *oneHandler == *eventHandler );
            } );
        }
        // использовать под залоченным для записи 'm_handlerListMutex'
        inline void deleteHandler( TEventHandlerIt it )
        {
            m_handlers.erase( it );
        }

        std::list<TMyEventHandlerPtr> m_handlers;

        // использовать под залоченным 'm_handlerListMutex'
        mutable TEventHandlerIt m_currentIt;
        mutable bool m_isCurrentItRemoved;

        mutable std::shared_mutex m_handlerListMutex;
};


} // events
