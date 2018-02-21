/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   TransactionManager.cpp
 * Author: Sergio
 * 
 * Created on 20 de febrero de 2018, 10:22
 */

#include "TransactionManager.h"

using namespace nlohmann;
using namespace medooze;

bool TransactionManager::handle(const std::string &str)
{
	//Process message
	json message = json::parse(str);
	
        //Check if it is correct
	if (message.find("type") == message.end())
	    //Ignore
	    return false;
            
	//Get type
	std::string type = message["type"];

	try
	{
		//Check type
		if (type=="cmd")
		{
			//Get cmd data
			std::string name	= message["name"];
			uint64_t  transId	= message["transId"];
			json data		= message["data"];
			std::string ns		= message["namespace"];

			//Create command
			auto cmd = std::make_shared<Command>(name,data,ns);

			//Set accept handler
			cmd->onaccepted([=](const json& data){
				//create response
				json response = {
				    {"type"	, "response"	},
				    {"transId"	, transId	},
				    {"data"	, data		},
				};

				//Send
				this->transport.send(response.dump());
			});
			
			//Set reject handler
			cmd->onrejected([=](const json& data){
				//create response
				json error = {
				    {"type"	, "error"	},
				    {"transId"	, transId	},
				    {"data"	, data		},
				};

				//Send
				this->transport.send(error.dump());
			});
			
			//If it has a namespace
			if (!ns.empty())
			{
				//Get associated namespace
				auto it = namespaces.find("ns");
				//If we have it
				if (it!=namespaces.end())
					//trigger event only on namespace
					it->second->emit(cmd);
				else
					//Launch event on main event handler
					emit(cmd);
			} else {
				//Launch event on main event handler
				emit(cmd);
			}
		} else if (type=="response") {
			//Get response data
			uint64_t transId	= message["transId"];
			json data		= message["data"];
			//Get transaction
			auto it = transactions.find(transId);
			//Check if fuound
			if (it==transactions.end())
				return false;
			//Get transaction
			auto transaction = it->second;
			//delete transacetion
			transactions.erase(it);
			//Accept it
			transaction->accept(data);
		} else if (type=="error") {
			//Get response data
			uint64_t transId	= message["transId"];
			json data		= message["data"];
			//Get transaction
			auto it = transactions.find(transId);
			//Check if fuound
			if (it==transactions.end())
				return false;
			//Get transaction
			auto transaction = it->second;
			//delete transacetion
			transactions.erase(it);
			//Accept it
			transaction->reject(data);
		} else if (type=="event") {
			//Get event data
			std::string name	= message["name"];
			json data		= message["data"];
			std::string ns		= message["namespace"];

			//Create event
			auto event = std::make_shared<Event>(name,data,ns);

			//If it has a namespace
			if (!ns.empty())
			{
				//Get associated namespace
				auto it = namespaces.find("ns");
				//If we have it
				if (it!=namespaces.end())
					//trigger event only on namespace
					it->second->emit(event);
				else
					//Launch event on main event handler
					emit(event);
			} else {
				//Launch event on main event handler
				emit(event);
			}
		} else {
			return false;
		}
	} catch (...) {
		return false;
	} 
	
	return true;
}

std::shared_ptr<Promise> TransactionManager::command(const std::string& name,  nlohmann::json data, const std::string& ns)
{
	//Get new transaction
	uint64_t transId = ++maxTransId;
	
	//create command
	json cmd = {
	    {"type"	, "cmd"		},
	    {"name"	, name		},
	    {"transId"	, transId	},
	    {"data"	, data		},
	    {"namespace", ns		},
	};
	//Create new promise
	auto promise = std::make_shared<Promise>();
	
	//Add to transactions
	transactions[transId] = promise;

	//Send
	this->transport.send(cmd.dump());
	
	//Return promise
	return promise;
}

void TransactionManager::event(const std::string& name,  nlohmann::json data, const std::string& ns)
{
	//create event
	json event = {
	    {"type"	, "event"	},
	    {"name"	, name		},
	    {"data"	, data		},
	    {"namespace", ns		},
	};

	//Send
	this->transport.send(event.dump());
}

std::shared_ptr<Namespace> TransactionManager::ns(const std::string &name)
{
	
	//Check if it was already present
	auto it = namespaces.find(name);
	//If found
	if (it!=namespaces.end())
		//Reuse it
		return it->second;
	
	//Create shared namespace
	auto ns = std::make_shared<Namespace>(this,name);
	//Put it on map
	namespaces[name] = ns;
	//DOne
	return ns;
}

std::shared_ptr<Promise> Namespace::command(const std::string& name, nlohmann::json data)
{
	//Call transaction name with our namespace
	return tm->command(name,data,ns);
}

void Namespace::event(const std::string& name,  nlohmann::json data)
{
	//Call transaction name with our namespace
	tm->event(name,data,ns);
}