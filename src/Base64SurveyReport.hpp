#pragma once

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <chrono>
#include <iomanip>

namespace b64_survey {
using namespace std;

using BenchmarkResults = map<string, map<int, double>>;

struct Base64SurveyReport
{
	static string Base64SurveyReport::GetCompilerSuffix();
	static string DumpJsonResult(const string &name, BenchmarkResults &results);

    template<class _ResultDelegate>
    static void PrintSortedResults(BenchmarkResults &results, _ResultDelegate &progress)
    {
        map<double, vector<string>> sorted;
        for (const auto &entry : results)
            sorted[entry.second.rbegin()->second].push_back(entry.first);

        MarkdownTable sortedTable{ 28, 10 };

        for (const pair<double, vector<string>> &entry : sorted)
        {
            for (const string &name : entry.second)
            {
                progress.onBeginSuite(name);
                map<int, double> suite = results[name];
                for (pair<int, double> testN : suite)
                {
                    progress.onCaseResult(testN.first, testN.second);
                }
            }
        }

        progress.onFinish();
    }
};


class MarkdownTable
{
private:
    vector<int> colWidths;
    bool m_hasHeader = false;
    int colIndex = 0;
    int width = 0;

public:
    explicit MarkdownTable(initializer_list<int> colWidths)
        : colWidths{ colWidths }
    {
    }

    int getColIndex(int index)
    {
        if (colWidths.empty())
            return 0;

        if (index < colWidths.size())
            return colWidths[index];

        return colWidths[colWidths.size() - 1];
    }

    MarkdownTable& endRow()
    {
        cout << "|" << endl;
        if (m_hasHeader == false)
        {
            m_hasHeader = true;
            for (int i = 0; i < width; ++i)
                cout << "|-" << left << setw(getColIndex(i)) << setfill('-') << "";
            cout << "|" << endl;
        }

        colIndex = 0;
        return *this;
    }

    template<class _Ty>
    MarkdownTable& col(const _Ty &colStr)
    {
        if (m_hasHeader == false)
            ++width;

        cout << "| " << right << setw(getColIndex(colIndex)) << setfill(' ') << setprecision(4) << fixed << colStr << flush;
        ++colIndex;

        return *this;
    }
};


class CSVResultsDelegate
{
public:
    explicit CSVResultsDelegate(const vector<int> &testCases)
    {
        cout << "Name";

        for (int size : testCases)
            cout << ",\t" << size;

    }

    void onBeginSuite(const string &name)
    {
        cout << endl;
        cout << name;
    }

    void onCaseResult(int caseIndex, double result)
    {
        cout << ",\t" << result << flush;
    }

    void onFinish()
    {
        cout << endl;
    }
};

class MarkdownResultsDelegate
{
private:
    MarkdownTable m_table{ 28, 10 };

public:
    explicit MarkdownResultsDelegate(const vector<int> &testCases)
    {
        m_table.col("Implementation");

        for (int size : testCases)
            m_table.col(size);
    }

    void onBeginSuite(const string &name)
    {
        m_table.endRow();
        m_table.col(name);
    }

    void onCaseResult(int caseIndex, double result)
    {
        m_table.col(result);
    }

    void onFinish()
    {
        m_table.endRow();
    }
};

} // namespace b64_survey
