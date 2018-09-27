#include <iostream>
#include <functional>
#include "eventhandling.hpp"


using namespace events;


class Foo
{
    public:
        Foo() :
            onMake( m_onMake ),
            m_onMake(),
            m_onMakeInner(),
            m_makeCount( 0 )
        {
            m_onMakeInner += FUNCTOR_HANDLER( m_onMake );
        }
        IEvent<unsigned int>& onMake;
        void make()
        {
            m_onMakeInner( m_makeCount++ );
        }
    private:
        TEvent<unsigned int> m_onMake, m_onMakeInner;
        unsigned int m_makeCount;
};


namespace instances
{
    Foo& getFoo()
    {
        static Foo _foo;
        return _foo;
    }
} // instances


struct FunctorHandler
{
    int operator()( unsigned int makeCount );
};

int functionHandler( unsigned int makeCount );

class ClassHandler
{
    public:
        int handle( unsigned int makeCount );
};


namespace instances
{
    FunctorHandler& getFunctorHandler()
    {
        static FunctorHandler _functorHandler;
        return _functorHandler;
    }
    std::function<int( unsigned int )>& getStdFunctionHandler()
    {
        static std::function<int( unsigned int )> _stdFunctionHandler = []( unsigned int makeCount )
        {
            std::cout << "It's std::function handler" << std::endl;
            if( makeCount >= 2 )
                instances::getFoo().onMake -= STD_FUNCTION_HANDLER( instances::getStdFunctionHandler() );
            return 7;
        };
        return _stdFunctionHandler;
    }
    ClassHandler& getClassHandler()
    {
        static ClassHandler _classHandler;
        return _classHandler;
    }
} // instances


int FunctorHandler::operator()( unsigned int makeCount )
{
    std::cout << "It's functor handler" << std::endl;
    if( makeCount >= 0 )
        instances::getFoo().onMake -= FUNCTOR_HANDLER( instances::getFunctorHandler() );

    return 6;
}

int functionHandler( unsigned int makeCount )
{
    std::cout << "It's function handler" << std::endl;
    if( makeCount >= 3 )
        instances::getFoo().onMake -= FUNCTION_HANDLER( functionHandler );

    return 9;
}

int ClassHandler::handle( unsigned int makeCount )
{
    std::cout << "It's method handler" << std::endl;
    if( makeCount >= 4 )
        instances::getFoo().onMake -= MY_METHOD_HANDLER( ClassHandler::handle );

    return 74;
}


int main( int argc, char* argv[] )
{
    Foo& foo = instances::getFoo();

    auto lambdaHandler = []( unsigned int )
    {
        std::cout << "It's lambda handler" << std::endl;
        return 12;
    };

    foo.onMake += FUNCTOR_HANDLER( instances::getFunctorHandler() );
    foo.onMake += LAMBDA_HANDLER( lambdaHandler );
    EventJoin lambdaJoin = foo.onMake += LAMBDA_HANDLER( ( [ &foo, &lambdaHandler ]( unsigned int makeCount )
    {
        if( makeCount >= 1 )
            foo.onMake -= LAMBDA_HANDLER( lambdaHandler );
        return 78;
    } ) );
    foo.onMake += STD_FUNCTION_HANDLER( instances::getStdFunctionHandler() );
    foo.onMake += FUNCTION_HANDLER( functionHandler );
    foo.onMake += METHOD_HANDLER( instances::getClassHandler(), ClassHandler::handle );

    for( int i = 0; i < 6; ++i )
    {
        std::cout << "Make " << i << " time:" << std::endl;
        foo.make();
        std::cout << std::endl;
    }

    lambdaJoin.unjoin();
   
    return 0;
}
