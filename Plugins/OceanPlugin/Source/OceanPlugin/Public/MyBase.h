// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include <functional>

namespace  OceanTool
{
	inline void CallBack(std::function<bool()> Condition,std::function<void()> Call)
	{
		auto Task = [Condition,Call](auto&& Task)->void
		{
			if(Condition())
			{
				AsyncTask(ENamedThreads::GameThread,[Call]{Call();});
			}
			else
			{
				AsyncTask(ENamedThreads::ActualRenderingThread,[Task]{Task(Task);});
			}
		};

		AsyncTask(ENamedThreads::ActualRenderingThread,[Task]{Task(Task);});
	}

	inline void CallBack(std::function<void()> Call)
	{
		AsyncTask(ENamedThreads::GameThread,[Call]{Call();});
	}

	inline  void BitReverse(std::vector<int>& Buffer)
	{
		int numOfBits = log2(Buffer.size());

		auto reverseBits = [numOfBits](int num) {
			int reversedNum = 0;
			for (int i = 0; i < numOfBits; i++) {
				// 检查 num 的第 i 位是否为1
				if ((num & (1 << i)) != 0) {
					// 如果为1，将 reversedNum 的第 (numOfBits - 1 - i) 位置为1
					reversedNum |= 1 << (numOfBits - 1 - i);
				}
			}
			return reversedNum;
		};
	
		for(int i = 0;i < Buffer.size();i++)
		{
			Buffer[i] = reverseBits(i);
		}
	}
}
