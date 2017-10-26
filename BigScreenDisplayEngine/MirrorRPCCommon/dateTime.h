#include <iostream>
#include <string>

#define BOOST_DATE_TIME_POSIX_TIME_STD_CONFIG
#include <boost/date_time/posix_time/posix_time.hpp>
using namespace boost::posix_time;
using namespace std;

class dateTime
{
public:
	void start()
	{
		startTime = microsec_clock::local_time();
	}

	void end()
	{
		endTime = microsec_clock::local_time();
	}

	long getTimeDuration()
	{
		time_duration td = (this->endTime - this->startTime);
		return td.total_milliseconds();
	}

	void showTimeDuration()
	{
		std::cout << "!!!!Time duration is: " << this->endTime - this->startTime << std::endl;
	}

	void showTimeDuration(unsigned long count)
	{
		std::cout << "Total times is: " << count << std::endl;
		std::cout << "Time duration is: " << this->endTime - this->startTime << std::endl;
		std::cout << "Time of once is: " << ((this->endTime - this->startTime).total_milliseconds()) / count << "milliseconds" << std::endl;
		std::cout << "Time of once is: " << ((this->endTime - this->startTime).total_microseconds()) / count << "microseconds" << std::endl;
		std::cout << "Time of once is: " << ((this->endTime - this->startTime).total_nanoseconds()) / count << "nanoseconds" << std::endl;
	}
	
private:
	ptime startTime;
	ptime endTime;
};