#include "binance.h"

Binance::Binance(const std::string &base_url, const std::string &coin)
    : base_url_(base_url), coin_(coin)
{
    // Setup connections
    curl_ = curl_easy_init();
    if (curl_)
    {
        struct curl_slist *headers_ = NULL;
        // headers_ = curl_slist_append(headers_, "accept: application/json/");
        headers_ = curl_slist_append(headers_, "content-Type: application/json");
        headers_ = curl_slist_append(headers_, "charsets: utf-8");

        curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers_);
        curl_easy_setopt(curl_, CURLOPT_URL, (base_url_ + "/api/v3/ticker/price?symbol=" + coin_).c_str());
        // curl_easy_setopt(curl_, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
        // curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 10);           // time-out
        // curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, Binance::writeData);
        curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &http_data_);
    }
}

void Binance::release()
{
    curl_easy_cleanup(curl_);
    curl_slist_free_all(headers_);
}

Binance::~Binance()
{
    release();
}

std::size_t Binance::writeData(const char *in, std::size_t size, std::size_t num, std::string *out)
{
    const std::size_t totalBytes(size * num);
    out->append(in, totalBytes);
    return totalBytes;
}

bool Binance::get()
{
    http_data_.clear();
    res_ = curl_easy_perform(curl_);
    if (res_ == CURLE_OK)
        curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &http_code_);

    if (http_code_ == 200)
    {
        if (json_reader_.parse(http_data_, json_data_))
        {
            double price(std::stod(json_data_["price"].asString()));
            cout << "\tPrice of " + coin_ << std::setprecision(10) << ": " << price << endl;
        }
        else
        {
            cout << "ERROR: Could not parse HTTP data as JSON" << endl;
            cout << "HTTP data was:\n"
                 << http_data_ << endl;
            return false;
        }
    }
    else
    {
        cout << "ERROR: http code: " << http_code_ << endl;
        return false;
    }
}