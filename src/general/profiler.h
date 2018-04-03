#ifndef PROFILER
#define PROFILER

#include <string>
#include <chrono>
#include <iostream>

using namespace std;

class Profiler
{
public:
    Profiler(string name, bool scoped = false)
    {
        _name = name;
        _scoped = scoped;
        _state = Waiting;
        if(scoped)
            begin();
    }

    ~Profiler()
    {
        if(_scoped)
            end();
    }

    void begin()
    {
        if(_state == Waiting)
        {
            _state = Active;
            _beginTime = chrono::high_resolution_clock::now();            
        }
    }

    void end()
    {
        if(_state == Active)
        {
            const auto now = chrono::high_resolution_clock::now();

            auto timeSpan = chrono::duration_cast<chrono::duration<double>>(now - this->_beginTime);
            _tick = timeSpan.count();
            _duration += timeSpan.count();

            _state = Waiting;

            if(_scoped)
                std::cout << getResult() << std::endl;
        }
    }

    string getResult()
    {
        string ret = "";
        ret.append(_name);
        ret.append(" - ");
        ret.append(to_string(_tick));
        ret.append(" Total: ");
        ret.append(to_string(_duration));
        ret.append(" s");
        return ret;
    }

    double _duration;
    double _tick;

    enum {
        Waiting,
        Active
    } _state;

    bool _scoped;
    chrono::high_resolution_clock::time_point _beginTime;
    string _name;
};

#endif // PROFILER

