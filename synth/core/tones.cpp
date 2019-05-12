#include "tones.h"

const TimbreModel& Sine()
{
	static const TimbreModel ret({ {1.,1.,waves::sine} });
	return ret;
}
const TimbreModel& Saw()
{
	static const TimbreModel ret({ {1.,1.,waves::sawtooth} });
	return ret;
}
const TimbreModel& Square()
{
	static const TimbreModel ret({ {1.,1.,waves::square} });
	return ret;
}
const TimbreModel& Triangle()
{
	static const TimbreModel ret({ {1.,1.,waves::triangle} });
	return ret;
}

const TimbreModel& Sines1()
{
	static const TimbreModel ret({
		{ 1., 1.,  waves::sine },
		{ 3., 0.3, waves::sine },
		{ 5., 0.3, waves::sine },
		{ 7., 0.3, waves::sine },
		{ 9., 0.3, waves::sine },
	});
	return ret;
};

const TimbreModel& Sines2()
{
	static const TimbreModel ret({
		{ 1., 1.,  waves::sine },
		{ 5., 0.2, waves::sine },
		{ 1/2., 0.1, waves::sine },
	});
	return ret;
};

const TimbreModel& SinesTriangles()
{
	static const TimbreModel ret({
		{ 1., 1.,  waves::sine },
		{ 3., 0.4, waves::triangle },
		{ 4., 0.1, waves::sine },
	});
	return ret;
};