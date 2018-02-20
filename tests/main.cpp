/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   mai.cpp
 * Author: Sergio
 *
 * Created on 20 de febrero de 2018, 15:29
 */

#include <stdlib.h>
#include <iostream>
#include <TransactionManager.h>

using namespace nlohmann;
using namespace medooze;

class Transport : public TransactionManager::Transport
{
public:
	TransactionManager* other;
	void send(const std::string& message) override
	{
		printf("sending:\r\n%s\r\n",message.c_str());
		other->handle(message);
	}
};

int main(int argc, char** argv)
{
	Transport t1;
	Transport t2;
	TransactionManager tm1(t1);
	TransactionManager tm2(t2);
	
	t1.other = &tm2;
	t2.other = &tm1;
	
	tm2.on<Command::shared>([&](Command::shared& cmd){
		//Get name
		const std::string& name = cmd->getName();
		
		//Check it
		if (name=="accept")
		{
			printf("t2::sending event 1\r\n");
			tm2.event("event1",{
				{"dummy",  "2"}
			});
			cmd->accept("accepted");
		} else if (name=="reject") {
			cmd->reject();
		} else {
			printf("we should not get here\r\n");
		}
		
	});
	
	printf("t1::sending accept cmd\r\n");
	auto cmd = tm1.command("accept", {{ "dummy", "1"}});
	cmd->onaccepted([](const json& data){
		printf("t1::command accepted");
	});

	printf("t1::sending reject cmd");
	auto reject = tm1.command("reject");
	reject->onrejected([](const json& data){
		printf("t1::command rejected");
	});

	auto ns1 = tm1.ns("ns");
	auto ns2 = tm2.ns("ns");

	ns2->on<Command::shared>([&](Command::shared& cmd){
		printf("ns2::got command %s\r\n", cmd->getName().c_str());
		cmd->accept("accepted");
	});

	ns2->on<Event::shared>([&](Event::shared& event){
		printf("ns2::got event %s\r\n", event->getName().c_str());
	});

	printf("ns1::sending test_namespace cmd");
	auto cmd2 = ns1->command("test_namespace", {{ "dummy", 1}});
	cmd2->onaccepted([](const json& data){
		printf("ns1::command accepted");
	});
	
	printf("ns1::sending test_namespace event");
	ns1->event("test_namespace", {{ "evt", 1}});
	ns1->event("test_namespace_empty_event");
	return 0;
}

