// Messenger.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Poco/Thread.h"
#include "Poco/Net/TCPServer.h"
#include "Poco/Net/TCPServerConnection.h"
#include "Poco/Net/TCPServerConnectionFactory.h"
#include "Poco/Net/TCPServerParams.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Timestamp.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/Exception.h"
#include "Poco/Util/ServerApplication.h"
#include <iostream>
#include <assert.h>

using namespace std;
using Poco::Net::ServerSocket;
using Poco::Net::StreamSocket;
using Poco::Net::TCPServerConnection;
using Poco::Net::TCPServerConnectionFactory;
using Poco::Net::TCPServer;
using Poco::Timestamp;
using Poco::DateTimeFormatter;
using Poco::DateTimeFormat;
using Poco::Util::ServerApplication;
using Poco::Util::Application;




struct MessageQueue{
	std::string from;
	std::string budy;
	MessageQueue *link ;
};
struct Person{
	std::string name;
	bool available;
	Person *Next;
	MessageQueue *FirstMessage;
};

std::string Refinestring(std::string temp)
{
	std::string refined="";
for(int i=0;i<temp.length();i++)
	{  		if (((char) temp[i])!='\b')
		refined=refined+((char) temp[i]);
	else
		if (refined.length()!=0)
			refined.resize(refined.length()-1);

}
return refined;
}
class MessagesClass 
{
private:
	Person *PersonHead;
public:
	MessagesClass()
	{
		//Head=new MessageQueue;
		PersonHead= new Person;
		PersonHead->Next=NULL;
		//Head->link=NULL;
	}
	bool OnlinePerson(std::string Name)
	{
		Person *TempPerson;
		bool Done=FALSE;
		bool Repetetive=FALSE;
		TempPerson=PersonHead;
		while (TempPerson->Next!=NULL)
		{
			if (TempPerson->Next->name.compare(Name)==0)
			{
				if(TempPerson->Next->available==FALSE)
				{
					Done=TRUE;
					TempPerson->Next->available=TRUE;
					break;
				}
				else
				{
					Done=TRUE;
					Repetetive=TRUE;
					break;
				}
			}
			TempPerson=TempPerson->Next;
		}
		if (Done==FALSE)
		{
			TempPerson->Next=new Person;
			TempPerson->Next->available=TRUE;
			TempPerson->Next->name=Name;
			TempPerson->Next->Next=NULL;
			TempPerson->Next->FirstMessage=new MessageQueue;
			TempPerson->Next->FirstMessage->link=NULL;
			return TRUE;
		}
		if (Done==TRUE && Repetetive==FALSE)
			return TRUE;
		else
			return FALSE;
	}
	void OfflinePerson(std::string Name)
	{
		Person *TempPerson;
		TempPerson=PersonHead;
		while (TempPerson->Next!=NULL)
		{
			if (TempPerson->Next->name.compare(Name)==0)
			{
				TempPerson->Next->available=FALSE;
				break;
			}
			TempPerson=TempPerson->Next;
		}

	}
	std::string	AnyMessage(std::string Name) const 
	{
		MessageQueue *TempQueue,*TempQueue1;
		TempQueue = new MessageQueue;
		Person *TempPerson;
		TempPerson=PersonHead;
		while (TempPerson->Next!=NULL)
		{
			if (TempPerson->Next->name.compare(Name)==0)
			{
				TempQueue=TempPerson->Next->FirstMessage;
				break;
			}
			TempPerson=TempPerson->Next;
		}


		std::string Message="";
		if (TempQueue->link!=NULL)
		{

			Message="User " + TempQueue->link->from + " Says:" +TempQueue->link->budy ; 
			if (TempQueue->link->link!=NULL)
			{
				TempQueue1=TempQueue->link;
				TempQueue->link=TempQueue->link->link;
				delete TempQueue1;
			}
			else
			{
				TempQueue1=TempQueue->link;
				TempQueue->link=NULL;
				delete TempQueue1;
			}

			return Message;

		}
		return "";
	}
	bool AddMessage(std::string from,std::string to,std::string budy)  
	{
		MessageQueue *TempQueue,*TempQueue1;
		Person *TempPerson;
		bool Done=FALSE;
		TempPerson=PersonHead;
		to.resize(to.find('\0')+1);
		TempQueue = new MessageQueue;
		while (TempPerson->Next!=NULL)
		{
			if (TempPerson->Next->name.compare(to)==0)
			{
				TempQueue=TempPerson->Next->FirstMessage;
				Done=TRUE;
				break;
			}
			TempPerson=TempPerson->Next;
		}
		if (!Done)
			return FALSE;

		TempQueue1 = new MessageQueue;
		//TempQueue->to=to;
		//TempQueue->to.resize(to.find('\0')+1);
		TempQueue1->from=from;
		TempQueue1->from.resize(from.find('\0')+1);
		TempQueue1->budy=budy;
		TempQueue1->budy.resize(budy.find('\0')+1);
		TempQueue1->link=TempQueue->link;
		TempQueue->link=TempQueue1;
		return TRUE;
	}
	void AddMessageForAll(std::string from,std::string budy)
	{
		Person *TempPerson;
		TempPerson=PersonHead;
		while (TempPerson->Next!=NULL)
		{
			if (TempPerson->Next->name.compare(from)!=0)
				if (TempPerson->Next->available==TRUE)
					AddMessage(from,TempPerson->Next->name,budy+'\0');
			TempPerson=TempPerson->Next;
		}
	}
	std::string AllOnlineUsers()const
	{
		Person *TempPerson;
		TempPerson=PersonHead;
		std::string temp="These Users are online : ";
		while (TempPerson->Next!=NULL)
		{
			if (TempPerson->Next->available==TRUE)
				temp = temp + TempPerson->Next->name + ',';

			TempPerson=TempPerson->Next;
		}
		return temp;
	}

};
MessagesClass *MessagePool= new MessagesClass;
class MessengerServerConnection: public TCPServerConnection
	/// This class handles all client connections.

{
public:
	MessengerServerConnection(const StreamSocket& s, const std::string& format): 
		TCPServerConnection(s)

	{
		_format=format;
		Username="";
	}

	void run()
	{
		Application& app = Application::instance();
		app.logger().information("Request from " + this->socket().peerAddress().toString());
		std::string reply="Hello buddy, Type your name and press enter please:\n\r";
		std::string Message="";
		std::string To;
		std::string Budy;
		char *tempString;

		char temp1[1]={' '};
		int fla=0;
		int len=1024;
		int len1=0;
		try
		{
			Timestamp now;
			std::string dt(DateTimeFormatter::format(now, _format));
			std::string StrTo;
			dt.append("\r\n");
			socket().sendBytes(dt.data(), (int) dt.length());
			socket().sendBytes(reply.data(), (int) reply.length());

			while (temp1[0]!='\n' )
			{
				len1=socket().receiveBytes(temp1,1,fla);
				if (len1!=0 && temp1[0]!='\r' && temp1[0]!='\n')
				{
					Username=Username + temp1[0];
				}
			}
			Username=Refinestring(Username+'\0');
			app.logger().information("This connection's name is: " + Username);
			if (MessagePool->OnlinePerson(Username))
			{
			MessagePool->AddMessageForAll(Username," I am online.\r\n");
			reply.clear();
			reply="Welcome " + Username +" "+ MessagePool->AllOnlineUsers()+"\r\n";
			socket().sendBytes(reply.data(), (int) reply.length());
			}
			else
			{
				reply.clear();
			reply="User " + Username +" is online via another terminal, please close the other one before connecting via another terminal"+ +"\r\n";
			socket().sendBytes(reply.data(), (int) reply.length());
			return;
			}

			this->socket().setBlocking(FALSE);
			temp1[0]=' ';
			Poco::Net::Socket::SocketList Readers,Writers,Exceptions;
			Poco::Timespan TimeSpan=0;
			while (TRUE)
			{   Poco::Thread::sleep(15);
				Readers.push_back(this->socket());

				while (this->socket().select(Readers,Writers,Exceptions,TimeSpan)==TRUE)
				{

					len1=socket().receiveBytes(temp1,1,fla);
					if (len1!=0)
					{
						StrTo=StrTo + temp1[0];
					}
					
					if (temp1[0]=='\n')
					{	StrTo=Refinestring(StrTo);
						if (StrTo.compare("exit\r\n")==0)
						{
							app.logger().information(Username + "Left The chatroom");
							MessagePool->OfflinePerson(Username);
							reply.clear();
							reply="I have gone offline, Goodbye everybody\r\n";
							MessagePool->AddMessageForAll(Username,reply);
							return;
						}
						std::size_t pos=StrTo.find(" ");
						if (pos>=1 && pos<=StrTo.length()){
							tempString=new char[pos];
							StrTo.copy(tempString,pos,0);
							To=tempString;
							To[pos]='\0';
							To.resize(To.find('\0')+1);
							StrTo.copy(tempString,StrTo.length()-pos,pos);
							Budy=tempString;
							Budy[StrTo.length()-pos]='\0';
							if(!MessagePool->AddMessage(Username,To,Budy))
							{
								To=To+" is not a user\r\n";
								socket().sendBytes(To.data(), (int) To.length());
							}
						}
						else
						{
							reply.clear();
							reply="The format of message shoud be: Username following by Message buddy, separated by a space \r\n";
							socket().sendBytes(reply.data(), (int) reply.length());
						}

						StrTo.clear();
						To.clear();

					}
				}
				if (Refinestring(StrTo)=="")
				Message=MessagePool->AnyMessage(Username);
				while (Message!="")
				{
					socket().sendBytes(Message.data(), (int) Message.length());
					Message=MessagePool->AnyMessage(Username);
				}

			}

		}
		catch (Poco::Exception& exc)
		{
			MessagePool->OfflinePerson(Username);
			app.logger().log(exc);
		}
	}

private:
	std::string Username;
	std::string _format;
};


class MessengerServerConnectionFactory: public TCPServerConnectionFactory
	/// A factory for TimeServerConnection.
{
public:
	MessengerServerConnectionFactory(const std::string& format):
		_format(format)
	{
	}

	TCPServerConnection* createConnection(const StreamSocket& socket)
	{
		return new MessengerServerConnection(socket, _format);
	}

private:
	std::string _format;
};

class MessengerServer: public Poco::Util::ServerApplication
	/// The main application class.
	///
	/// This class handles command-line arguments and
	/// configuration files.
	/// Start the MessengerServer executable 
	///
	/// To use the sample configuration file (MessengerServer.properties),
	/// copy the file to the directory where the MessengerServer executable
	/// resides. If you start the debug version of the MessengerServer
	/// (MessengerServer[.exe]), you must also create a copy of the configuration
	/// file named MessengerServer.properties. In the configuration file, you
	/// can specify the port on which the server is listening (default
	/// 9900) and some other simple configurations
	///
	/// To test the MessengerServer you can use any telnet client (telnet localhost 9900).
	/// write your user name press enter and to send message you need to write the username of the person you want to chat following by the message divided by a space.
	/// to exit , simply write exit
{
public:
	MessengerServer()
	{
	}

	~MessengerServer()
	{
	}

protected:
	void initialize(Application& self)
	{
		loadConfiguration(); // load default configuration files, if present
		ServerApplication::initialize(self);
	}

	void uninitialize()
	{
		ServerApplication::uninitialize();
	}


	int main(const std::vector<std::string>& args)
	{


		{
			// get parameters from configuration file
			unsigned short port = (unsigned short) config().getInt("MessengerServer.port", 9900);
			std::string format(config().getString("MessengerServerTime.format", DateTimeFormat::ISO8601_FORMAT));

			// set-up a server socket
			ServerSocket svs(port);
			// set-up a TCPServer instance
			TCPServer srv(new MessengerServerConnectionFactory(format), svs);

			//some tests for the MessageQueue and PersonQueue 
			//for (int i=1;i<100;i++)
			//	{
			//		assert (TRUE==MessagePool->OnlinePerson(to_string(i)+'\0'));
			//		for (int j=1;j<100;j++)
			//		MessagePool->AddMessage(to_string(j)+'\0',to_string(i)+'\0',"Hello budddy how are you"+to_string(j*i)+"\r\n"+'\0');
			//	}
			//for (int i=1;i<100;i++)
			//	{
			//		for (int j=1;j<100;j++)
			//					MessagePool->AnyMessage(to_string(i)+'\0');
			//		
			//	}
			//for (int i=1;i<100;i++)
			//{
			//assert(MessagePool->AnyMessage(to_string(i)+'\0')=="");
			//}
			// start the TCPServer
			srv.start();
			// wait for CTRL-C or kill
			waitForTerminationRequest();
			// Stop the TCPServer
			srv.stop();
		}
		return Application::EXIT_OK;
	}

};




int _tmain(int argc, _TCHAR* argv[])
{
	MessengerServer server;
	return server.run(argc, argv);

	return 0;
}

