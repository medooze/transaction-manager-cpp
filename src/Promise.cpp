/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Promise.cpp
 * Author: Sergio
 * 
 * Created on 20 de febrero de 2018, 17:25
 */

#include "Promise.h"

using namespace medooze;

std::function<void (const nlohmann::json& data)> Promise::noop = [](const nlohmann::json& data){};

Promise& Promise::onaccepted(const std::function<void (const nlohmann::json& data)> &callback) 
{ 
	switch (state)
	{
		case State::Pending:
			//Store callback for later
			accepted = callback;
			break;
		case State::Resolved:
			//Call it now
			callback(resolved);
			break;
		default:
			//Bad luck
			break;
	}
		
	return *this;
}

Promise& Promise::onrejected(const std::function<void (const nlohmann::json& data)> &callback)
{
	switch (state)
	{
		case State::Pending:
			//Store callback for later
			rejected = callback;
			break;
		case State::Rejected:
			//Call it now
			callback(resolved);
			break;
		default:
			//Bad luck
			break;
	}
	
	return *this;
}

//Accet or reject command
bool Promise::accept(const nlohmann::json& data)
{ 
	//Check state
	if (state!=State::Pending)
		return false;
	//Resolved
	state = State::Resolved;
	//Copy data
	resolved = nlohmann::json(data);
	//Call callback
	accepted(resolved); 
	//Ok
	return true;
}

bool Promise::reject(const nlohmann::json& data)
{ 
	//Check state
	if (state!=State::Pending)
		return false;
	//Rejected
	state = State::Rejected;
	//Copy data
	resolved = nlohmann::json(data);
	//Call callback
	rejected(resolved); 
	//Ok
	return true;
}
