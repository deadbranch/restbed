/*
 * Copyright (c) 2013, 2014, 2015 Corvusoft
 */

//System Includes
#include <map>
#include <thread>
#include <string>
#include <memory>
#include <ciso646>
#include <stdexcept>
#include <functional>

//Project Includes
#include <restbed>

//External Includes
#include <catch.hpp>

//System Namespaces
using std::thread;
using std::string;
using std::multimap;
using std::shared_ptr;
using std::make_shared;

//Project Namespaces
using namespace restbed;

//External Namespaces

void head_handler( const shared_ptr< Session > session )
{
    session->close( 200, "Hello, World!", { { "Content-Length", "13" }, { "Connection", "close" } } );
}

SCENARIO( "publishing single path resources", "[resource]" )
{
    auto resource = make_shared< Resource >( );
    resource->set_path( "/resources/1" );
    resource->set_method_handler( "HEAD", head_handler );
    
    auto settings = make_shared< Settings >( );
    settings->set_port( 1984 );
    
    shared_ptr< thread > worker = nullptr;
    
    Service service;
    service.publish( resource );
    service.set_ready_handler( [ &worker ]( Service & service )
    {
        worker = make_shared< thread >( [ &service ] ( )
        {
            GIVEN( "I publish a resource at '/resources/1' with a HTTP 'HEAD' method handler" )
            {
                WHEN( "I perform a HTTP 'HEAD' request to '/resources/1'" )
                {
                    Request request;
                    request.set_port( 1984 );
                    request.set_host( "localhost" );
                    request.set_method( "HEAD" );
                    request.set_path( "/resources/1" );
                    
                    auto response = Http::sync( request );
                    
                    THEN( "I should see a '200' (OK) status code" )
                    {
                        REQUIRE( 200 == response->get_status_code( ) );
                    }
                    
                    AND_THEN( "I should see a repsonse body of 'Hello, World!'" )
                    {
                        Bytes expection { 'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!' };
                        REQUIRE( response->get_body( ) == expection );
                    }

                    multimap< string, string > headers = response->get_headers( );
                    
                    AND_THEN( "I should see a 'Connection' header value of 'close'" )
                    {
                        auto header = headers.find( "Connection" );
                        REQUIRE( header not_eq headers.end( ) );
                        REQUIRE( "close" == headers.find( "Connection" )->second );
                    }
                    
                    AND_THEN( "I should see a 'Content-Length' header value of '13'" )
                    {
                        auto header = headers.find( "Content-Length" );
                        REQUIRE( header not_eq headers.end( ) );
                        REQUIRE( "13" == headers.find( "Content-Length" )->second );
                    }
                }
                
                service.stop( );
            }
        } );
    } );
    
    service.start( settings );
    worker->join( );
}
