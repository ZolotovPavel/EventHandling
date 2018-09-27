#pragma once

#include <memory>
#include <assert.h>
#include "abstracteventhandler.hpp"


namespace events {

namespace handlers {


namespace
{

    template<class TMethodHolder, class ...TParams>
    struct IsMethodParamsCompatible
    {
        private:

            template<class TCheckedMethodHolder, class ...TCheckedParams>
            static constexpr std::true_type exists( decltype( ( std::declval<TCheckedMethodHolder>().m_object.*std::declval<TCheckedMethodHolder>().m_method )( std::declval<TCheckedParams>()... ) )* = nullptr ) noexcept;
            
            template<class TCheckedMethodHolder, class ...TCheckedParams>
            static constexpr std::false_type exists( ... ) noexcept;

        public:

            static constexpr bool value = decltype( exists<TMethodHolder, TParams...>( nullptr ) )::value;
    };

} //


template<class TMethodHolder, class ...TParams>
class MethodEventHandler : public AbstractEventHandler<TParams...>
{
    using MyType = MethodEventHandler<TMethodHolder, TParams...>;
    using TMethodHolderPtr = std::shared_ptr<TMethodHolder>;

    public:

        MethodEventHandler( TMethodHolderPtr methodHolder ) :
            AbstractEventHandler<TParams...>(),
            m_methodHolder( methodHolder )
        {
            assert( m_methodHolder != nullptr );
        }

        virtual void call( TParams... params ) override
        {
            static_assert( IsMethodParamsCompatible<TMethodHolder, TParams...>::value, "Event and method arguments are not compatible" );

            ( m_methodHolder->m_object.*m_methodHolder->m_method )( params... );
        }

    protected:

        virtual bool isEquals( const AbstractEventHandler<TParams...>& other ) const noexcept override
        {
            const MyType* _other = dynamic_cast<const MyType*>( &other );
            return ( _other != nullptr && *m_methodHolder == *_other->m_methodHolder );
        }

    private:

        TMethodHolderPtr m_methodHolder;
};


template<class TObject, class TResult, class ...TParams>
class MethodHolder
{
    using MyType = MethodHolder<TObject, TResult, TParams...>;
    using TMethod = TResult( TObject::* )( TParams... );

    public:

        template<class ...TCallParams>
        operator TEventHandlerPtr<TCallParams...>()
        {
            return TEventHandlerPtr<TCallParams...>( new MethodEventHandler<MyType, TCallParams...>( m_me.lock() ) );
        }

        bool operator==( const MyType& other ) const noexcept
        {
            return ( &m_object == &other.m_object && m_method == other.m_method );
        }
        bool operator!=( const MyType& other ) const noexcept
        {
            return !( *this == other );
        }

        template<class TObject, class ...TParams>
        static std::shared_ptr<MyType> create( TObject& object, TMethod method )
        {
            std::shared_ptr<MyType> result( new MyType( object, method ) );
            result->m_me = result;
            return result;
        }

    private:

        MethodHolder( TObject& object, TMethod method ) :
            m_object( object ),
            m_method( method )
        {
            assert( m_method != nullptr );
        }

        TObject& m_object;
        TMethod m_method;

        std::weak_ptr<MyType> m_me;

    template<class TMethodHolder, class ...TParams> friend class MethodEventHandler;
    template<class TMethodHolder, class ...TParams> friend struct IsMethodParamsCompatible;
};


template<class TObject, class TResult, class ...TParams>
std::shared_ptr<MethodHolder<TObject, TResult, TParams...>> createMethodEventHandler( TObject& object, TResult( TObject::*method )( TParams... ) )
{
    return MethodHolder<TObject, TResult, TParams...>::create( object, method );
}


} // handlers

} // events


#define     METHOD_HANDLER( Object, Method )     ::events::handlers::createMethodEventHandler( Object, &Method )
#define     MY_METHOD_HANDLER( Method )          METHOD_HANDLER( *this, Method )
