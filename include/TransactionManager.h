/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   TransactionManager.h
 * Author: Sergio
 *
 * Created on 20 de febrero de 2018, 10:22
 */

#ifndef TRANSACTIONMANAGER_H
#define TRANSACTIONMANAGER_H
#include <functional>
#include <vector>
#include <map>
#include <json.hpp>
#include <EventEmitter.h>
#include "Promise.h"


namespace medooze
{

class TransactionManager;

class Message
{
public:
	Message(const std::string& name, const nlohmann::json& data, const std::string& ns) :
		name(name),
		data(data),
		ns(ns)
	{}
		
	//Getterrs
	const std::string&	getName()	{ return name;	}
	const nlohmann::json&	getData()	{ return data;	}
	const std::string&	getNamespace()	{ return ns;	}
	
private:
	std::string    name;
	nlohmann::json data;
	std::string    ns;	
};

class Event :
	public Message
{
public:
	//Alias
	typedef std::shared_ptr<Event> shared;
public:
	Event(const std::string& name, const nlohmann::json& data, const std::string& ns) 
		: Message(name, data, ns) 
	{}
};

class Command :
	public Message,
	public Promise
{
public:
	//Alias
	typedef std::shared_ptr<Command> shared;
	
public:
	Command(const std::string& name, const nlohmann::json& data, const std::string& ns)
		: Message(name, data, ns) 
	{}
};

class Namespace : public EventEmitter
{
public:
	typedef std::shared_ptr<Namespace> shared;
public:
	Namespace(TransactionManager *tm, const std::string& ns) :
		tm(tm),
		ns(ns)
	{}

	Promise::shared command(const std::string& name, nlohmann::json data = nlohmann::json());
	void event(const std::string& name,  nlohmann::json data = nlohmann::json());
	const std::string& getName() { return ns; }
private:
	TransactionManager *tm;
	std::string ns;
};

class TransactionManager : public EventEmitter
{
public:
	class Transport
	{
	public:
		virtual void send(const std::string& message) = 0;
	};
public:
	TransactionManager(Transport& transport) : 
		transport(transport)
	{}
	
	Promise::shared   command(const std::string& name, nlohmann::json data = nlohmann::json(), const std::string& ns = "");
	void		  event(const std::string& name, nlohmann::json data = nlohmann::json(), const std::string& ns = "");
	Namespace::shared ns(const std::string &name);
	
	bool handle(const std::string& message);
private:
	uint64_t maxTransId = 0;
	Transport& transport;
	std::map<std::string,std::shared_ptr<Namespace>> namespaces;
	std::map<uint64_t,std::shared_ptr<Promise>> transactions;
	
};

}; // namespace medooze
#endif /* TRANSACTIONMANAGER_H */

