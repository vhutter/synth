#include "tones.h"

const TimbreModel& Sine13()
{
	static const TimbreModel ret(
		{
		TimbreModel::ToneSkeleton{ 1., 1.,  waves::sine },
		TimbreModel::ToneSkeleton{ 3., 0.3, waves::sine },
		TimbreModel::ToneSkeleton{ 5., 0.3, waves::sine },
		TimbreModel::ToneSkeleton{ 7., 0.3, waves::sine },
		TimbreModel::ToneSkeleton{ 9., 0.3, waves::sine },
		}
	);
	return ret;
};