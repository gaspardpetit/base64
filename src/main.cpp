#include <chrono>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <filesystem>
#include <sstream>

#ifdef _WIN32
#include <Windows.h>
#endif // _WIN32

//other
using namespace std::chrono;
using namespace std;

std::map<std::string, std::string(*)(string &)> &getEncodeTests()
{
    static map<string, string(*)(string &)> s_tests;
    return s_tests;
}

std::map<std::string, std::string(*)(string &)> &getDecodeTests()
{
    static map<string, string(*)(string &)> s_tests;
    return s_tests;
}

string s_tiny = "Hey";
string s_tiny_expected = "SGV5";
std::string s_short = "Hello World!!!";
std::string s_short_expected = "SGVsbG8gV29ybGQhISE=";
std::string s_medium =
R"(Lorem ipsum dolor sit amet, consectetur adipiscing elit. Etiam ex nunc, rhoncus in blandit at, rutrum sed turpis. Aenean in bibendum dolor, vitae facilisis dolor. Quisque imperdiet et nulla non feugiat. Fusce elementum est eu nibh efficitur aliquet. Quisque elementum diam libero, eget auctor nunc condimentum in.)";
std::string s_medium_expected =
R"(TG9yZW0gaXBzdW0gZG9sb3Igc2l0IGFtZXQsIGNvbnNlY3RldHVyIGFkaXBpc2NpbmcgZWxpdC4gRXRpYW0gZXggbnVuYywgcmhvbmN1cyBpbiBibGFuZGl0IGF0LCBydXRydW0gc2VkIHR1cnBpcy4gQWVuZWFuIGluIGJpYmVuZHVtIGRvbG9yLCB2aXRhZSBmYWNpbGlzaXMgZG9sb3IuIFF1aXNxdWUgaW1wZXJkaWV0IGV0IG51bGxhIG5vbiBmZXVnaWF0LiBGdXNjZSBlbGVtZW50dW0gZXN0IGV1IG5pYmggZWZmaWNpdHVyIGFsaXF1ZXQuIFF1aXNxdWUgZWxlbWVudHVtIGRpYW0gbGliZXJvLCBlZ2V0IGF1Y3RvciBudW5jIGNvbmRpbWVudHVtIGluLg==)";
std::string s_long =
R"(Lorem ipsum dolor sit amet, consectetur adipiscing elit. Etiam ex nunc, rhoncus in blandit at, rutrum sed turpis. Aenean in bibendum dolor, vitae facilisis dolor. Quisque imperdiet et nulla non feugiat. Fusce elementum est eu nibh efficitur aliquet. Quisque elementum diam libero, eget auctor nunc condimentum in. Maecenas sit amet est maximus, ultricies quam ac, sagittis odio. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Vestibulum pharetra nisl non lacus pharetra lobortis. Vivamus vel tortor ac nulla lobortis scelerisque in sit amet enim.
Lorem ipsum dolor sit amet, consectetur adipiscing elit. Integer vitae erat accumsan quam eleifend suscipit. Nullam finibus mi a lacus lobortis vehicula. Nulla facilisi. Quisque molestie odio et nulla scelerisque, in maximus turpis posuere. Suspendisse magna quam, imperdiet ac diam eget, pharetra fringilla eros. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Proin ac ex sapien. Praesent accumsan consequat sapien, vel pretium ligula hendrerit in. Aenean sit amet ligula nec leo tincidunt egestas. Nulla ullamcorper eros sed risus venenatis, ut eleifend nunc vulputate. Praesent egestas nisi sed quam gravida, a tempor sem interdum. Nam a arcu nibh.
Integer vulputate nunc ac erat sollicitudin porta id id risus. Fusce nec aliquam risus, at tincidunt sapien. Suspendisse dignissim, erat eu rutrum mollis, lacus lorem lacinia enim, nec ornare nibh justo commodo nibh. Proin velit mi, dapibus non tincidunt id, pretium in metus. Morbi eu rhoncus tortor, ac tempor lacus. In vehicula non purus eu faucibus. Vestibulum volutpat nunc id pharetra posuere. Sed sagittis ligula in fermentum viverra. Aliquam aliquet sed nisi non sodales. Duis ullamcorper urna quam, in imperdiet urna ullamcorper quis. In hac habitasse platea dictumst.
Donec cursus, felis a pulvinar tempus, elit mi hendrerit felis, et volutpat eros felis sed lacus. Nulla est ipsum, molestie sed nisl in, sagittis laoreet ex. Curabitur dapibus egestas lobortis. Morbi pulvinar placerat tellus sed sollicitudin. Ut finibus suscipit mi, at fermentum purus tincidunt eu. Nulla suscipit magna vel massa tincidunt laoreet at at odio. Fusce nec magna eros.
Interdum et malesuada fames ac ante ipsum primis in faucibus. Nam condimentum, velit dictum scelerisque tincidunt, arcu libero fermentum libero, lacinia hendrerit lorem tellus quis lectus. Donec placerat sodales eros rutrum tincidunt. Aliquam erat volutpat. Interdum et malesuada fames ac ante ipsum primis in faucibus. Praesent id nulla at justo rutrum ultrices. Vestibulum hendrerit elit eget pharetra porta. Nunc euismod velit nulla, a faucibus purus venenatis sed. Donec id augue lobortis purus vestibulum fermentum. Suspendisse nibh nisi, malesuada in pulvinar et, pretium blandit turpis. Interdum et malesuada fames ac ante ipsum primis in faucibus. Etiam at mollis ex. Cras in nunc quis lacus aliquam laoreet. Ut tincidunt nisl ac sapien dictum, id suscipit elit mattis. Pellentesque tristique interdum lobortis.)";
std::string s_long_expected =
R"(TG9yZW0gaXBzdW0gZG9sb3Igc2l0IGFtZXQsIGNvbnNlY3RldHVyIGFkaXBpc2NpbmcgZWxpdC4gRXRpYW0gZXggbnVuYywgcmhvbmN1cyBpbiBibGFuZGl0IGF0LCBydXRydW0gc2VkIHR1cnBpcy4gQWVuZWFuIGluIGJpYmVuZHVtIGRvbG9yLCB2aXRhZSBmYWNpbGlzaXMgZG9sb3IuIFF1aXNxdWUgaW1wZXJkaWV0IGV0IG51bGxhIG5vbiBmZXVnaWF0LiBGdXNjZSBlbGVtZW50dW0gZXN0IGV1IG5pYmggZWZmaWNpdHVyIGFsaXF1ZXQuIFF1aXNxdWUgZWxlbWVudHVtIGRpYW0gbGliZXJvLCBlZ2V0IGF1Y3RvciBudW5jIGNvbmRpbWVudHVtIGluLiBNYWVjZW5hcyBzaXQgYW1ldCBlc3QgbWF4aW11cywgdWx0cmljaWVzIHF1YW0gYWMsIHNhZ2l0dGlzIG9kaW8uIFBlbGxlbnRlc3F1ZSBoYWJpdGFudCBtb3JiaSB0cmlzdGlxdWUgc2VuZWN0dXMgZXQgbmV0dXMgZXQgbWFsZXN1YWRhIGZhbWVzIGFjIHR1cnBpcyBlZ2VzdGFzLiBWZXN0aWJ1bHVtIHBoYXJldHJhIG5pc2wgbm9uIGxhY3VzIHBoYXJldHJhIGxvYm9ydGlzLiBWaXZhbXVzIHZlbCB0b3J0b3IgYWMgbnVsbGEgbG9ib3J0aXMgc2NlbGVyaXNxdWUgaW4gc2l0IGFtZXQgZW5pbS4KTG9yZW0gaXBzdW0gZG9sb3Igc2l0IGFtZXQsIGNvbnNlY3RldHVyIGFkaXBpc2NpbmcgZWxpdC4gSW50ZWdlciB2aXRhZSBlcmF0IGFjY3Vtc2FuIHF1YW0gZWxlaWZlbmQgc3VzY2lwaXQuIE51bGxhbSBmaW5pYnVzIG1pIGEgbGFjdXMgbG9ib3J0aXMgdmVoaWN1bGEuIE51bGxhIGZhY2lsaXNpLiBRdWlzcXVlIG1vbGVzdGllIG9kaW8gZXQgbnVsbGEgc2NlbGVyaXNxdWUsIGluIG1heGltdXMgdHVycGlzIHBvc3VlcmUuIFN1c3BlbmRpc3NlIG1hZ25hIHF1YW0sIGltcGVyZGlldCBhYyBkaWFtIGVnZXQsIHBoYXJldHJhIGZyaW5naWxsYSBlcm9zLiBMb3JlbSBpcHN1bSBkb2xvciBzaXQgYW1ldCwgY29uc2VjdGV0dXIgYWRpcGlzY2luZyBlbGl0LiBQcm9pbiBhYyBleCBzYXBpZW4uIFByYWVzZW50IGFjY3Vtc2FuIGNvbnNlcXVhdCBzYXBpZW4sIHZlbCBwcmV0aXVtIGxpZ3VsYSBoZW5kcmVyaXQgaW4uIEFlbmVhbiBzaXQgYW1ldCBsaWd1bGEgbmVjIGxlbyB0aW5jaWR1bnQgZWdlc3Rhcy4gTnVsbGEgdWxsYW1jb3JwZXIgZXJvcyBzZWQgcmlzdXMgdmVuZW5hdGlzLCB1dCBlbGVpZmVuZCBudW5jIHZ1bHB1dGF0ZS4gUHJhZXNlbnQgZWdlc3RhcyBuaXNpIHNlZCBxdWFtIGdyYXZpZGEsIGEgdGVtcG9yIHNlbSBpbnRlcmR1bS4gTmFtIGEgYXJjdSBuaWJoLgpJbnRlZ2VyIHZ1bHB1dGF0ZSBudW5jIGFjIGVyYXQgc29sbGljaXR1ZGluIHBvcnRhIGlkIGlkIHJpc3VzLiBGdXNjZSBuZWMgYWxpcXVhbSByaXN1cywgYXQgdGluY2lkdW50IHNhcGllbi4gU3VzcGVuZGlzc2UgZGlnbmlzc2ltLCBlcmF0IGV1IHJ1dHJ1bSBtb2xsaXMsIGxhY3VzIGxvcmVtIGxhY2luaWEgZW5pbSwgbmVjIG9ybmFyZSBuaWJoIGp1c3RvIGNvbW1vZG8gbmliaC4gUHJvaW4gdmVsaXQgbWksIGRhcGlidXMgbm9uIHRpbmNpZHVudCBpZCwgcHJldGl1bSBpbiBtZXR1cy4gTW9yYmkgZXUgcmhvbmN1cyB0b3J0b3IsIGFjIHRlbXBvciBsYWN1cy4gSW4gdmVoaWN1bGEgbm9uIHB1cnVzIGV1IGZhdWNpYnVzLiBWZXN0aWJ1bHVtIHZvbHV0cGF0IG51bmMgaWQgcGhhcmV0cmEgcG9zdWVyZS4gU2VkIHNhZ2l0dGlzIGxpZ3VsYSBpbiBmZXJtZW50dW0gdml2ZXJyYS4gQWxpcXVhbSBhbGlxdWV0IHNlZCBuaXNpIG5vbiBzb2RhbGVzLiBEdWlzIHVsbGFtY29ycGVyIHVybmEgcXVhbSwgaW4gaW1wZXJkaWV0IHVybmEgdWxsYW1jb3JwZXIgcXVpcy4gSW4gaGFjIGhhYml0YXNzZSBwbGF0ZWEgZGljdHVtc3QuCkRvbmVjIGN1cnN1cywgZmVsaXMgYSBwdWx2aW5hciB0ZW1wdXMsIGVsaXQgbWkgaGVuZHJlcml0IGZlbGlzLCBldCB2b2x1dHBhdCBlcm9zIGZlbGlzIHNlZCBsYWN1cy4gTnVsbGEgZXN0IGlwc3VtLCBtb2xlc3RpZSBzZWQgbmlzbCBpbiwgc2FnaXR0aXMgbGFvcmVldCBleC4gQ3VyYWJpdHVyIGRhcGlidXMgZWdlc3RhcyBsb2JvcnRpcy4gTW9yYmkgcHVsdmluYXIgcGxhY2VyYXQgdGVsbHVzIHNlZCBzb2xsaWNpdHVkaW4uIFV0IGZpbmlidXMgc3VzY2lwaXQgbWksIGF0IGZlcm1lbnR1bSBwdXJ1cyB0aW5jaWR1bnQgZXUuIE51bGxhIHN1c2NpcGl0IG1hZ25hIHZlbCBtYXNzYSB0aW5jaWR1bnQgbGFvcmVldCBhdCBhdCBvZGlvLiBGdXNjZSBuZWMgbWFnbmEgZXJvcy4KSW50ZXJkdW0gZXQgbWFsZXN1YWRhIGZhbWVzIGFjIGFudGUgaXBzdW0gcHJpbWlzIGluIGZhdWNpYnVzLiBOYW0gY29uZGltZW50dW0sIHZlbGl0IGRpY3R1bSBzY2VsZXJpc3F1ZSB0aW5jaWR1bnQsIGFyY3UgbGliZXJvIGZlcm1lbnR1bSBsaWJlcm8sIGxhY2luaWEgaGVuZHJlcml0IGxvcmVtIHRlbGx1cyBxdWlzIGxlY3R1cy4gRG9uZWMgcGxhY2VyYXQgc29kYWxlcyBlcm9zIHJ1dHJ1bSB0aW5jaWR1bnQuIEFsaXF1YW0gZXJhdCB2b2x1dHBhdC4gSW50ZXJkdW0gZXQgbWFsZXN1YWRhIGZhbWVzIGFjIGFudGUgaXBzdW0gcHJpbWlzIGluIGZhdWNpYnVzLiBQcmFlc2VudCBpZCBudWxsYSBhdCBqdXN0byBydXRydW0gdWx0cmljZXMuIFZlc3RpYnVsdW0gaGVuZHJlcml0IGVsaXQgZWdldCBwaGFyZXRyYSBwb3J0YS4gTnVuYyBldWlzbW9kIHZlbGl0IG51bGxhLCBhIGZhdWNpYnVzIHB1cnVzIHZlbmVuYXRpcyBzZWQuIERvbmVjIGlkIGF1Z3VlIGxvYm9ydGlzIHB1cnVzIHZlc3RpYnVsdW0gZmVybWVudHVtLiBTdXNwZW5kaXNzZSBuaWJoIG5pc2ksIG1hbGVzdWFkYSBpbiBwdWx2aW5hciBldCwgcHJldGl1bSBibGFuZGl0IHR1cnBpcy4gSW50ZXJkdW0gZXQgbWFsZXN1YWRhIGZhbWVzIGFjIGFudGUgaXBzdW0gcHJpbWlzIGluIGZhdWNpYnVzLiBFdGlhbSBhdCBtb2xsaXMgZXguIENyYXMgaW4gbnVuYyBxdWlzIGxhY3VzIGFsaXF1YW0gbGFvcmVldC4gVXQgdGluY2lkdW50IG5pc2wgYWMgc2FwaWVuIGRpY3R1bSwgaWQgc3VzY2lwaXQgZWxpdCBtYXR0aXMuIFBlbGxlbnRlc3F1ZSB0cmlzdGlxdWUgaW50ZXJkdW0gbG9ib3J0aXMu)";

using BenchmarkResult = std::map<std::string, std::map<int, double>>;


static inline std::string getSuffix()
{
#if defined(_MSC_VER)
    int ver = _MSC_VER / 100;
    std::stringstream strm;
    strm << "msvc" << ver;
    return strm.str();
#elif defined(__clang_major__)
    int ver = __clang_major__;
    std::stringstream strm;
    strm << "clang" << ver;
    return strm.str();
#else
    return "unknown";
#endif
}

static const std::string resultDir = "result/";
static const std::string encodePath = resultDir + "encode-" + getSuffix() + ".js";
static const std::string decodePath = resultDir + "decode-" + getSuffix() + ".js";

static int test_sizes[] = { 20, 60, 100, 300, 1000, 2000, 4000, 9000, 16000, 30000, 60000 };

void DumpJsonResult(const std::string &path, BenchmarkResult &results,
    const std::string &varName)
{

    std::ofstream outFile(path);
    if (outFile)
    {
        //
        outFile << "var " << varName << " = [\n";
        //
        uint32_t cnt = 0;
        for (auto i : results)
        {
            // get
            auto [name, statVal] = i;
            if (cnt > 0)
                outFile << ",\n";

            cnt++;
            outFile << "\t{name:'" << name << "',\n";
            outFile << "\tvalues:[\n";

            uint32_t rcnt = 0;
            for (auto j : statVal)
            {
                auto [len, t] = j;
                if (rcnt > 0)
                    outFile << ",\n";
                rcnt++;
                outFile << "\t\t{len:" << len << ",time:" << t << "}";
            }
            outFile << "\n\t]}";
        }
        outFile << "]\n";
        outFile.close();
    }
    else
    {
        cout << "can not create file " << path << "\n";
    }
}

void RunEncodeBenchmark()
{
    bool printTable = true;
    typedef high_resolution_clock Clock;
    std::map<std::string, std::map<int, double>> results;
    string buffers[sizeof(test_sizes) / sizeof(test_sizes[0])];
    string value_buffers[sizeof(test_sizes) / sizeof(test_sizes[0])];
    for (int i = 0; i < sizeof(test_sizes) / sizeof(test_sizes[0]); ++i)
    {
        buffers[i].resize(test_sizes[i]);
        for (int j = 0; j < test_sizes[i]; ++j)
            buffers[i][j] = rand();
    }

    for (int i = 0; i < sizeof(test_sizes) / sizeof(test_sizes[0]); ++i)
    {
        value_buffers[i] = getEncodeTests().begin()->second(buffers[i]);
    }

    if (printTable)
    {
        cout << "| Implementation ";

        for (int size : test_sizes)
            cout << "| " << size;

        cout << "|" << endl;

        cout << "|----";

        for (int size : test_sizes)
            cout << "|----";

        cout << "|" << endl;
    }
    else
    {
        cout << "Name";

        for (int size : test_sizes)
            cout << ",\t" << size;

        cout << endl;
    }

    for (const auto &entry : getEncodeTests())
    {
        const string &name = entry.first;

        if (printTable)
        {
            cout << "| " << name;
        }
        else
        {
            cout << name;
        }

        for (int testN = 0; testN < sizeof(test_sizes) / sizeof(test_sizes[0]);
            ++testN)
        {

            // run N iterations, or break after 0.5s
            int i;
            auto t1 = Clock::now();
            for (i = 0; i < 1000000; ++i)
            {
                (entry.second)(buffers[testN]);
                if (duration_cast<milliseconds>(Clock::now() - t1).count() >= 500)
                    break;
            }
            auto t2 = Clock::now();
            double time_tiny =
                double(duration_cast<microseconds>(t2 - t1).count()) / double(i + 1);
            results[name][test_sizes[testN]] = time_tiny;
            if (printTable)
            {
                cout << "| " << time_tiny << std::flush;
            }
            else
            {
                cout << ",\t" << time_tiny << std::flush;
            }
        }

        if (printTable)
            cout << "|" << endl;
        else
            cout << endl;
    }

    std::map<double, std::vector<std::string>> sorted;
    for (const auto &entry : results)
        sorted[entry.second.rbegin()->second].push_back(entry.first);

    if (printTable)
    {
        cout << "| Implementation ";

        for (int size : test_sizes)
            cout << "| " << size;

        cout << "|" << endl;

        cout << "|----";

        for (int size : test_sizes)
            cout << "|----";

        cout << "|" << endl;
    }

    for (const auto &entry : sorted)
    {
        for (const std::string &name : entry.second)
        {
            if (printTable)
            {
                cout << "| " << name;
            }
            else
            {
                cout << name;
            }

            for (int testN = 0; testN < sizeof(test_sizes) / sizeof(test_sizes[0]);
                ++testN)
            {
                double time_tiny = results[name][test_sizes[testN]];
                results[name][test_sizes[testN]] = time_tiny;
                if (printTable)
                {
                    cout << "| " << time_tiny << std::flush;
                }
                else
                {
                    cout << ",\t" << time_tiny << std::flush;
                }
            }

            if (printTable)
                cout << "|" << endl;
            else
                cout << endl;
        }
    }
    // print sorted summary

    // dump results to csv for futher processing
    DumpJsonResult(encodePath, results, "encode_result_" + getSuffix());

    // create stat javascript files
}

void RunDecodeBenchmark()
{
    bool printTable = true;
    typedef high_resolution_clock Clock;
    std::map<std::string, std::map<int, double>> results;
    string buffers[sizeof(test_sizes) / sizeof(test_sizes[0])];
    string value_buffers[sizeof(test_sizes) / sizeof(test_sizes[0])];
    for (int i = 0; i < sizeof(test_sizes) / sizeof(test_sizes[0]); ++i)
    {
        buffers[i].resize(test_sizes[i]);
        for (int j = 0; j < test_sizes[i]; ++j)
            buffers[i][j] = rand();
    }

    for (int i = 0; i < sizeof(test_sizes) / sizeof(test_sizes[0]); ++i)
    {
        value_buffers[i] = getEncodeTests().begin()->second(buffers[i]);
    }

    if (printTable)
    {
        cout << "| Implementation ";

        for (int size : test_sizes)
            cout << "| " << size;

        cout << "|" << endl;

        cout << "|----";

        for (int size : test_sizes)
            cout << "|----";

        cout << "|" << endl;
    }
    else
    {
        cout << "Name";

        for (int size : test_sizes)
            cout << ",\t" << size;

        cout << endl;
    }

    for (const auto &entry : getDecodeTests())
    {
        const string &name = entry.first;

        if (printTable)
        {
            cout << "| " << name;
        }
        else
        {
            cout << name;
        }

        for (int testN = 0; testN < sizeof(test_sizes) / sizeof(test_sizes[0]);
            ++testN)
        {
            std::string result = (entry.second)(value_buffers[testN]);
            if (result != buffers[testN])
            {
                cout << "failure, expected " << buffers[testN] << " but got " << result
                    << endl;
                break;
            }

            // run N iterations, or break after 0.5s
            int i;
            auto t1 = Clock::now();
            for (i = 0; i < 1000000; ++i)
            {
                (entry.second)(value_buffers[testN]);
                if (duration_cast<milliseconds>(Clock::now() - t1).count() >= 500)
                    break;
            }
            auto t2 = Clock::now();
            double time_tiny =
                double(duration_cast<microseconds>(t2 - t1).count()) / double(i + 1);
            results[name][test_sizes[testN]] = time_tiny;
            if (printTable)
            {
                cout << "| " << time_tiny << std::flush;
            }
            else
            {
                cout << ",\t" << time_tiny << std::flush;
            }
        }

        if (printTable)
            cout << "|" << endl;
        else
            cout << endl;
    }

    std::map<double, std::vector<std::string>> sorted;
    for (const auto &entry : results)
        sorted[entry.second.rbegin()->second].push_back(entry.first);

    if (printTable)
    {
        cout << "| Implementation ";

        for (int size : test_sizes)
            cout << "| " << size;

        cout << "|" << endl;

        cout << "|----";

        for (int size : test_sizes)
            cout << "|----";

        cout << "|" << endl;
    }

    for (const auto &entry : sorted)
    {
        for (const std::string &name : entry.second)
        {
            if (printTable)
            {
                cout << "| " << name;
            }
            else
            {
                cout << name;
            }

            for (int testN = 0; testN < sizeof(test_sizes) / sizeof(test_sizes[0]);
                ++testN)
            {
                double time_tiny = results[name][test_sizes[testN]];
                results[name][test_sizes[testN]] = time_tiny;
                if (printTable)
                {
                    cout << "| " << time_tiny << std::flush;
                }
                else
                {
                    cout << ",\t" << time_tiny << std::flush;
                }
            }

            if (printTable)
                cout << "|" << endl;
            else
                cout << endl;
        }
    }

    // print sorted summary
    DumpJsonResult(decodePath, results, "decode_result_" + getSuffix());
}

int RunTests(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return testing::UnitTest::GetInstance()->Run();
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
    // try to get more consistent results
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
#endif // _WIN32
    //
    auto runTest = [](const std::string &in, const std::string &expected) {
    };


    // create result directory
    filesystem::create_directory(resultDir);

    //
    int result = RunTests(argc + 1, argv);
    cout << "Press enter to continue." << endl;
    getchar();
    RunEncodeBenchmark();
    RunDecodeBenchmark();
    cout << "Press enter to continue." << endl;
    getchar();
    return result;
}
