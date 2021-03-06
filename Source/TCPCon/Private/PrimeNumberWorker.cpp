// Fill out your copyright notice in the Description page of Project Settings.

#include "PrimeNumberWorker.h"




//***********************************************************
//Thread Worker Starts as NULL, prior to being instanced
//		This line is essential! Compiler error without it
FPrimeNumberWorker* FPrimeNumberWorker::Runnable = NULL;
//***********************************************************

FPrimeNumberWorker::FPrimeNumberWorker(TArray<int32>& TheArray, const int32 IN_TotalPrimesToFind, ATCPlayerController* IN_PC)
	: ThePC(IN_PC)
	, TotalPrimesToFind(IN_TotalPrimesToFind)
	, StopTaskCounter(0)
	, PrimesFoundCount(0)
{
	//Link to where data should be stored
	PrimeNumbers = &TheArray;

	Thread = FRunnableThread::Create(this, TEXT("FPrimeNumberWorker"), 0, TPri_BelowNormal); //windows default = 8mb for thread, could specify more
}

FPrimeNumberWorker::~FPrimeNumberWorker()
{
	delete Thread;
	Thread = NULL;
}

//Init
bool FPrimeNumberWorker::Init()
{
	//Init the Data 
	PrimeNumbers->Empty();
	PrimeNumbers->Add(2);
	PrimeNumbers->Add(3);

	if (ThePC)
	{
		ThePC->ClientMessage("**********************************");
		ThePC->ClientMessage("Prime Number Thread Started!");
		ThePC->ClientMessage("**********************************");
	}
	return true;
}

//Run
uint32 FPrimeNumberWorker::Run()
{
	//Initial wait before starting
	FPlatformProcess::Sleep(0.03);

	//While not told to stop this thread 
	//		and not yet finished finding Prime Numbers
	while (StopTaskCounter.GetValue() == 0 && !IsFinished())
	{
		PrimeNumbers->Add(FindNextPrimeNumber());
		PrimesFoundCount++;

		//***************************************
		//Show Incremental Results in Main Game Thread!

		//	Please note you should not create, destroy, or modify UObjects here.
		//	  Do those sort of things after all thread are completed.

		//	  All calcs for making stuff can be done in the threads
		//	     But the actual making/modifying of the UObjects should be done in main game thread.
		ThePC->ClientMessage(FString::FromInt(PrimeNumbers->Last()));
		//***************************************

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//prevent thread from using too many resources
		//FPlatformProcess::Sleep(0.01);
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	}

	//Run FPrimeNumberWorker::Shutdown() from the timer in Game Thread that is watching
	//to see when FPrimeNumberWorker::IsThreadFinished()

	return 0;
}

//stop
void FPrimeNumberWorker::Stop()
{
	StopTaskCounter.Increment();
}

FPrimeNumberWorker* FPrimeNumberWorker::JoyInit(TArray<int32>& TheArray, const int32 IN_TotalPrimesToFind, ATCPlayerController* IN_PC)
{
	//Create new instance of thread if it does not exist
	//		and the platform supports multi threading!
	if (!Runnable && FPlatformProcess::SupportsMultithreading())
	{
		Runnable = new FPrimeNumberWorker(TheArray, IN_TotalPrimesToFind, IN_PC);
	}
	return Runnable;
}

void FPrimeNumberWorker::EnsureCompletion()
{
	Stop();
	Thread->WaitForCompletion();
}

void FPrimeNumberWorker::Shutdown()
{
	if (Runnable)
	{
		Runnable->EnsureCompletion();
		delete Runnable;
		Runnable = NULL;
	}
}

bool FPrimeNumberWorker::IsThreadFinished()
{
	if (Runnable) return Runnable->IsFinished();
	return true;
}
int32 FPrimeNumberWorker::FindNextPrimeNumber()
{
	//Last known prime number  + 1
	int32 TestPrime = PrimeNumbers->Last();

	bool NumIsPrime = false;
	while (!NumIsPrime)
	{
		NumIsPrime = true;

		//Try Next Number
		TestPrime++;

		//Modulus from 2 to current number - 1 
		for (int32 b = 2; b < TestPrime; b++)
		{
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			//prevent thread from using too many resources
			//FPlatformProcess::Sleep(0.01);
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

			if (TestPrime % b == 0)
			{
				NumIsPrime = false;
				break;
				//~~~
			}
		}
	}

	//Success!
	return TestPrime;
}