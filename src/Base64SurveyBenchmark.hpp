#pragma once

#include "Base64SurveyRegistry.hpp"
#include "Base64SurveyReport.hpp"

#include <vector>
#include <chrono>

namespace b64_survey {
using namespace std;
using namespace std::chrono;

struct Base64SurveyBenchmark
{
    template<typename _FN>
    static pair<int, microseconds> RunUntil(int maxIter, chrono::milliseconds maxMS, _FN &&fn)
    {
        // run N iterations, or break after 0.5s
        using Clock = chrono::high_resolution_clock;
        time_point t1 = Clock::now();

        int iter = 0;
        while (iter++ < maxIter)
        {
            fn();

            if (Clock::now() - t1 >= maxMS)
                break;
        }

        chrono::time_point t2 = Clock::now();
        chrono::microseconds elapsed = duration_cast<chrono::microseconds>(t2 - t1);
        return make_pair(iter, elapsed);
    }

    template<class _ProgressTy>
    static BenchmarkResults RunEncodeBenchmark(const vector<int> &testCases, _ProgressTy &progress, int maxIter, milliseconds maxMS)
    {
        BenchmarkResults results;
        vector<string> buffers{ testCases.size() };
        vector<string> value_buffers{ testCases.size() };
        for (int i = 0; i < testCases.size(); ++i)
        {
            buffers[i].resize(testCases[i]);
            for (int j = 0; j < testCases[i]; ++j)
                buffers[i][j] = rand();
        }

        for (int i = 0; i < testCases.size(); ++i)
        {
            value_buffers[i] = Base64SurveyRegistry::GetEncoders().begin()->second(buffers[i]);
        }

        for (const pair<string, Base64EncoderProc> &entry : Base64SurveyRegistry::GetEncoders())
        {
            const string &name = entry.first;
            progress.onBeginSuite(name);

            for (int testN = 0; testN < testCases.size(); ++testN)
            {
                pair<int, microseconds> result = RunUntil(maxIter, maxMS, [&]() {
                    (entry.second)(buffers[testN]);
                    });
                double time_tiny = double(result.second.count()) / double(result.first);
                results[name][testCases[testN]] = time_tiny;
                progress.onCaseResult(testN, time_tiny);
            }
        }

        progress.onFinish();
        return results;
    }

    template<class _ProgressTy>
    static BenchmarkResults RunDecodeBenchmark(const std::vector<int> &testCases, _ProgressTy &progress, int maxIter, milliseconds maxMS)
    {
        BenchmarkResults results;
        std::vector<string> buffers{ testCases.size() };
        std::vector<string> value_buffers{ testCases.size() };
        for (int i = 0; i < testCases.size(); ++i)
        {
            buffers[i].resize(testCases[i]);
            for (int j = 0; j < testCases[i]; ++j)
                buffers[i][j] = rand();
        }

        for (int i = 0; i < testCases.size(); ++i)
        {
            value_buffers[i] = Base64SurveyRegistry::GetEncoders().begin()->second(buffers[i]);
        }

        for (const pair<string, Base64DecoderProc> &entry : Base64SurveyRegistry::GetDecoders())
        {
            const string &name = entry.first;
            progress.onBeginSuite(name);

            for (int testN = 0; testN < testCases.size(); ++testN)
            {
                std::string decodeResult = (entry.second)(value_buffers[testN]);
                if (decodeResult != buffers[testN])
                {
                    cout << "failure, expected " << buffers[testN] << " but got " << decodeResult
                        << endl;
                    break;
                }

                pair<int, microseconds> result = RunUntil(maxIter, maxMS, [&]() {
                    (entry.second)(value_buffers[testN]);
                    });

                double time_tiny = double(result.second.count()) / double(result.first);
                results[name][testCases[testN]] = time_tiny;
                progress.onCaseResult(testN, time_tiny);
            }
        }

        progress.onFinish();
        return results;
    }
};

} // namespace b64_survey
