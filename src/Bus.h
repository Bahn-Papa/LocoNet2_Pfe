
#pragma once

//##########################################################################
//#
//#		Bus.h
//#
//#	The classes in this file will provide the basic functionality to take
//#	LocoNet messages from several sources and resend them to many
//#	destinations (observer design pattern).
//#	This can be used to resend messages between "physical" LocoNet,
//#	LocoNet over USB / Serial / Ethernet / ...
//#
//##########################################################################


//==========================================================================
//
//		I N C L U D E S
//
//==========================================================================

#define ETL_NO_STL
#include <Embedded_Template_Library.h> // Mandatory for Arduino IDE only
#include <etl/vector.h>

#include "ln_opc.h"

#define BUS_DEBUG_

#ifdef BUS_DEBUG
#include <Arduino.h>
#define BUS_DEBUGF(format, ...)  do{ log_printf(ARDUHAL_LOG_FORMAT(I, format), ##__VA_ARGS__); }while(0)
#else
#define BUS_DEBUGF(...)
#endif


//==========================================================================
//
//		C L A S S   D E F I N I T I O N S
//
//==========================================================================


////////////////////////////////////////////////////////////////////////
//
//	CLASS:	Consumer
//
//----------------------------------------------------------------------
//
//	Each class that wants to receive (consume) LocoNet messages must be
//	derived from this class (observer class, will observe the 'Bus').
//
template < class Msg, class Ret >
class Consumer
{
	public:
		virtual Ret onMessage( const Msg &msg ) = 0;
};


////////////////////////////////////////////////////////////////////////
//
//	CLASS:	Bus
//
//----------------------------------------------------------------------
//
//	This is the "big" distributor (object to be observed).
//	Whenever there is a new LocoNet message it will be 'broadcast'
//	to all participants on the 'Bus'.
//
template < class Msg, class Ret, Ret okVal, const size_t MAX_CONSUMERS >
class Bus
{
	public:
		using MsgConsumer = Consumer<Msg, Ret>;

		Ret broadcast(const Msg &msg, MsgConsumer* sender = nullptr)
		{
			BUS_DEBUGF( "message %02x %02x...", msg.data[ 0 ], msg.data[ 1 ] );

			Ret ret = okVal;

			for( const auto &aConsumer : consumers )
			{
				//--------------------------------------------------
				//	prevent looping of the LocoNet message:
				//	do not deliver the message to the partisipant
				//	who starts the broadcast.
				//
				if( sender != aConsumer )
				{
					Ret error = aConsumer->onMessage( msg );

					//----------------------------------------------
					//	store the last error
					//
					if( error != okVal )
					{
						ret = error;
					}
				}
			}

			return( ret );
		}

		void addConsumer( MsgConsumer *pConsumer )
		{
			consumers.push_back( pConsumer );
		}

		void removeConsumer( MsgConsumer *pConsumer )
		{
			consumers.erase( etl::remove( consumers.begin(), consumers.end(), pConsumer ), consumers.end() );
		}

	private:
		etl::vector< MsgConsumer *, MAX_CONSUMERS > consumers;
};

