#ifndef PROMISE_H
#define PROMISE_H
#include <json.hpp>

namespace medooze
{
	
class Promise 
{
public:
	
	enum State 
	{
		Pending,
		Resolved,
		Rejected
	};
	typedef std::shared_ptr<Promise> shared;
public:
	static std::function<void (const nlohmann::json& data)> noop;
public:
	explicit Promise() = default;
	Promise& onaccepted(std::function<void (const nlohmann::json& data)> callback);
	Promise& onrejected(std::function<void (const nlohmann::json& data)> callback);

	//Accet or reject command
	bool accept(const nlohmann::json& data = nlohmann::json());
	bool reject(const nlohmann::json& data = nlohmann::json());
	
private:
	std::function<void (const nlohmann::json& data)> &accepted = Promise::noop;
	std::function<void (const nlohmann::json& data)> &rejected = Promise::noop;
	State state = State::Pending;
	nlohmann::json resolved;
};

}//
#endif /* PROMISE_H */

