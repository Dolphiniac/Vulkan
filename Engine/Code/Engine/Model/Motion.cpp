#include "Engine/Model/Motion.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Model/Skeleton.hpp"
#include "Engine/Model/AnimationCurve.hpp"
#include "Engine/Core/BinaryReader.hpp"
#include "Engine/Core/BinaryWriter.hpp"
#include "Engine/Core/ConsoleCommand.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Actor/Actor.hpp"

#include <math.h>


//-----------------------------------------------------------------------------------------------
Motion* loadedMotion = nullptr;


// //-----------------------------------------------------------------------------------------------
// Motion::Motion(int jointCount, Matrix44* keyFrames, float totalSeconds, float frameRate)
// 	: m_jointCount(jointCount)
// 	, m_totalLengthSeconds(totalSeconds)
// 	, m_frameRate(frameRate)
// {
// 	m_frameTime = 1.f / frameRate;
// 	m_frameCount = (int)ceil(frameRate * m_totalLengthSeconds) + 1;
// 	m_keyFrames = (Matrix44*)malloc(sizeof(Matrix44) * jointCount * m_frameCount);
// 	memcpy(m_keyFrames, keyFrames, sizeof(Matrix44) * jointCount * m_frameCount);
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// void Motion::GetFrameIndicesWithBlend(size_t& outFrameIndex0, size_t& outFrameIndex1, float& outBlend, float inTime)
// {
// 	outFrameIndex0 = (size_t)floor(inTime / m_frameTime);
// 	outFrameIndex1 = outFrameIndex0 + 1;
// 
// 	if (outFrameIndex0 == (m_frameCount - 1))
// 	{
// 		outFrameIndex1 = outFrameIndex0;
// 		outBlend = 0.f;
// 	}
// 	else if (outFrameIndex0 == (m_frameCount - 2))
// 	{
// 		float lastFrameTime = m_totalLengthSeconds - (m_frameTime * outFrameIndex0);
// 		outBlend = fmodf(inTime, m_frameTime) / m_frameTime;
// 		outBlend = Clampf(outBlend, 0.f, 1.f);
// 	}
// 	else
// 	{
// 		outBlend = fmodf(inTime, m_frameTime) / m_frameTime;
// 	}
// }


//-----------------------------------------------------------------------------------------------
//void Motion::ApplyMotionToSkeleton(class Skeleton* skeleton, float time)
//{
//	size_t frame0;
//	size_t frame1;
//	float blend;
//	GetFrameIndicesWithBlend(frame0, frame1, blend, time);
//
//	size_t jointCount = skeleton->GetNumJoints();
//	for (int jointIndex = 0; jointIndex < jointCount; jointIndex++)
//	{
//		Matrix44& mat0 = GetJointKeyFrame(jointIndex, frame0);
//		Matrix44& mat1 = GetJointKeyFrame(jointIndex, frame1);
//
//		Matrix44 newModel = Lerp(mat0, mat1, blend);
//		skeleton->SetJointWorldTransform(jointIndex, newModel);
//	}
//}


//-----------------------------------------------------------------------------------------------
void Motion::Update(class Skeleton* skeleton, float deltaSeconds)
{
	UNUSED(skeleton);
	if (!m_isPlaying)
	{
		return;
	}
	m_totalTime += deltaSeconds;
	float simulationTimeForPlaybackMode = m_totalTime;

	if (simulationTimeForPlaybackMode > m_totalLengthOfAnimation + startTime)
	{
		switch (m_mode)
		{
		case CLAMP:
			simulationTimeForPlaybackMode = m_totalLengthOfAnimation + startTime;
			break;
		case LOOPING:
			while (simulationTimeForPlaybackMode > m_totalLengthOfAnimation + startTime)
			{
				simulationTimeForPlaybackMode -= m_totalLengthOfAnimation + startTime;
			}
			break;
		case PINGPONG:
			while (simulationTimeForPlaybackMode > (m_totalLengthOfAnimation + startTime) * 2.f)
			{
				//Iterate down to a range of 0 to 2*totalLength
				simulationTimeForPlaybackMode -= (m_totalLengthOfAnimation + startTime) * 2.f;
			}
			if (simulationTimeForPlaybackMode > m_totalLengthOfAnimation + startTime)
			{
				//If time > totalLength, we need to play in reverse
				simulationTimeForPlaybackMode = (m_totalLengthOfAnimation + startTime) * 2.f - simulationTimeForPlaybackMode;
			}
		}
	}

	//ApplyMotionToSkeleton(skeleton, simulationTimeForPlaybackMode);
}


//-----------------------------------------------------------------------------------------------
std::vector<Matrix44> Motion::GetMatricesForSkeletonAtNormalizedTime(class Skeleton* skeleton, float normalizedTime)
{
	float time = normalizedTime * (m_totalLengthOfAnimation + startTime);
	std::vector<Matrix44> result;
	int jointCount = skeleton->GetNumJoints();
	result.reserve(jointCount);
	int curveCount = m_curves.size();
	if (curveCount < jointCount)
	{
		//If this skeleton has more joints than the source skeleton, animate up to the number of curves we have,
		//then update the rest with 0 delta so they follow the skeleton but don't animate.
		int jointIndex;
		for (jointIndex = 0; jointIndex < curveCount; jointIndex++)
		{
			Joint* joint = skeleton->GetJoint(jointIndex);
			Matrix44 localTransformChangeForJoint = m_curves[jointIndex]->EvaluateLocalTransformAt(time, skeleton->m_jointMetadata[jointIndex]);
			result.push_back(localTransformChangeForJoint);
			//skeleton->SetWorldTransformForJoint(jointIndex, localTransformChangeForJoint);
			delete joint;
		}
		for (; jointIndex < jointCount; jointIndex++)
		{
			result.push_back(Matrix44::Identity);
			//skeleton->SetWorldTransformForJoint(jointIndex, Matrix44::Identity);
		}
	}
	else
	{
		for (int jointIndex = 0; jointIndex < jointCount; jointIndex++)
		{
			Joint* joint = skeleton->GetJoint(jointIndex);
			Matrix44 localTransformChangeForJoint = m_curves[jointIndex]->EvaluateLocalTransformAt(time, skeleton->m_jointMetadata[jointIndex]);
			result.push_back(localTransformChangeForJoint);
			//skeleton->SetWorldTransformForJoint(jointIndex, localTransformChangeForJoint);
			delete joint;
		}
	}
	//If this skeleton has fewer joints than the source, then animate the joints we have, and don't use the remaining animation curves

	return result;
	//skeleton->ApplyGeometricTransforms();
}


//-----------------------------------------------------------------------------------------------
void Motion::WriteToFile(const std::string& filepath)
{
	BinaryWriter writer;

	writer.Write(m_version);
	writer.Write(m_totalLengthOfAnimation);
	writer.Write(startTime);
	writer.Write((int)m_curves.size());
	for (AnimationCurve* curve : m_curves)
	{
		curve->SaveToWriter(writer);
	}

	writer.WriteBufferToFile(filepath);
}


//-----------------------------------------------------------------------------------------------
Motion* Motion::ReadFromFile(const std::string& filepath)
{
	BinaryReader reader(filepath);

	int version;
	float totalAnimationLength;
	reader.Read(version);
	reader.Read(totalAnimationLength);

	Motion* result = new Motion(totalAnimationLength);
	result->m_version = version;
	reader.Read(result->startTime);
	int numCurves;
	reader.Read(numCurves);

	for (int i = 0; i < numCurves; i++)
	{
		AnimationCurve* curve = AnimationCurve::CreateFromReader(reader);
		result->AddAnimationCurve(curve);
	}

	return result;
}


CONSOLE_COMMAND(MotionSave, args)
{
	std::string filename = args.GetNextArg();
	std::vector<std::string> actorMotionPair = SplitOnDelimiter(filename, '@');
	GUARANTEE_OR_DIE(actorMotionPair.size() == 2, "Bad actor motion pair");
	std::string filepath = "Data/Actors/" + actorMotionPair[0] + "/" + actorMotionPair[1] + ".anim";
	if (!loadedMotion)
	{
		ConsolePrint("Cannot save motion that does not exist!", RED);
		return;
	}

	loadedMotion->WriteToFile(filepath);
	ConsolePrintf(WHITE, "Success!  Motion saved to %s", filepath.c_str());
}

CONSOLE_COMMAND(MotionLoad, args)
{
	std::string filename = args.GetNextArg();
	std::vector<std::string> actorMotionPair = SplitOnDelimiter(filename, '@');
	std::string filepath = "Data/Actors/" + actorMotionPair[0] + "/" + actorMotionPair[1] + ".anim";
	if (FindFilesWith("Data/Actors/" + actorMotionPair[0], actorMotionPair[1] + ".anim").empty())
	{
		ConsolePrintf(RED, "File %s does not exist", filepath.c_str());
		return;
	}
	loadedMotion = Motion::ReadFromFile(filepath);
}

extern Actor* loadedActor;
CONSOLE_COMMAND(Motion, args)
{
	std::string arg = args.GetNextArg();
	
	while (arg != "")
	{
		if (arg == "set")
		{
			arg = args.GetNextArg();
			if (loadedActor)
			{
				auto motionIter = loadedActor->m_motions.find(arg);
				bool foundMotion = motionIter != loadedActor->m_motions.end();
				if (foundMotion)
				{
					loadedMotion = motionIter->second;
				}
				else
				{
					ConsolePrintf(RED, "Motion %s not found", arg.c_str());
				}
			}
			else
			{
				ConsolePrint("\"set\" command is not valid if there is no loaded actor", RED);
			}
		}
		else if (arg == "mode")
		{
			if (!loadedMotion)
			{
				ConsolePrint("Motion commands invalid while no motion loaded", RED);
				return;
			}
			std::string mode = args.GetNextArg();
			if (mode == "clamp")
			{
				loadedMotion->SetPlayMode(CLAMP);
			}
			else if (mode == "loop" || mode == "looping")
			{
				loadedMotion->SetPlayMode(LOOPING);
			}
			else if (mode == "pingpong")
			{
				loadedMotion->SetPlayMode(PINGPONG);
			}
			else if (arg == "")
			{
				ConsolePrint("Must supply argument to \"mode\".  Accepted arguments are \"clamp\", \"looping\", or \"pingpong\"", RED);
				return;
			}
			else
			{
				ConsolePrintf(RED, "Invalid argument to \"mode\": %s.  Accepted arguments are \"clamp\", \"looping\", or \"pingpong\"", mode.c_str());
				return;
			}
		}
		else if (arg == "reset")
		{
			if (!loadedMotion)
			{
				ConsolePrint("Motion commands invalid while no motion loaded", RED);
				return;
			}
			loadedMotion->ResetAnimation();
		}
		else if (arg == "play")
		{
			if (!loadedMotion)
			{
				ConsolePrint("Motion commands invalid while no motion loaded", RED);
				return;
			}
			loadedMotion->Play();
		}
		else if (arg == "pause")
		{
			if (!loadedMotion)
			{
				ConsolePrint("Motion commands invalid while no motion loaded", RED);
				return;
			}
			loadedMotion->Pause();
		}
		else
		{
			ConsolePrintf(RED, "Unsupported motion command: %s.  Accepted commands are \"mode\", \"play\", \"pause\", and \"reset\"", arg.c_str());
		}
		arg = args.GetNextArg();
	}
}


//-----------------------------------------------------------------------------------------------
Motion::~Motion()
{
	for (AnimationCurve* ac : m_curves)
	{
		delete ac;
	}
}