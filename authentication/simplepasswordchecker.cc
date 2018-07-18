#include "simplepasswordchecker.hh"

using namespace std;

SimplePasswordChecker::SimplePasswordChecker(const std::string &user, const std::string &password)
	: user(user), password(password)
{
	if (    user.size()     < 1 || user.size()     > 255 ||
		password.size() < 1 || password.size() > 255)
	{
		//TODO: proper exception
		throw exception();
	}
}

bool SimplePasswordChecker::check(const string &user, const string &password)
{
	return this->user == user && this->password == password;
}