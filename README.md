# http-request
[![build](https://github.com/Nevermore1994/http-request/workflows/build/badge.svg)](https://github.com/Nevermore1994/http-request/actions)
### Overview
This repository provides a framework for handling HTTP requests and responses. It includes structures for request information, error information, response headers, and a response handler to manage different stages of the HTTP communication process. Additionally, it provides a Request class to initiate and manage HTTP requests.

### Features

    • Cross-Platform: This framework is designed to be cross-platform, allowing you to use it on various operating systems.
    • Modern C++: Requires C++17 or above, leveraging modern C++ features for better performance and readability.
    • Flexible and Extensible: Easily extend and customize the request and response handling to fit your needs.
    • HTTPS Support: Supports HTTPS for secure communication.
    • Thread-Safe: Designed to be thread-safe for concurrent use.

### Structures

#### RequestInfo
The RequestInfo structure holds the information necessary to make an HTTP request.
```c++
struct RequestInfo {
    /// Indicates if redirects are allowed. Default is true.
    bool isAllowRedirect = true;

    /// Specifies the IP version. Default is IPVersion::Auto.
    IPVersion ipVersion = IPVersion::Auto;

    /// The URL to which the request is made.
    std::string url;

    /// The HTTP method type (Get, Post, etc.). Default is HttpMethodType::Unknown.
    HttpMethodType methodType = HttpMethodType::Unknown;

    /// A map of HTTP headers to include in the request.
    std::unordered_map<std::string, std::string> headers;

    /// The body of the request. Default is nullptr.
    DataRefPtr body = nullptr;

    /// The timeout duration for the request. Default is 60 seconds.
    std::chrono::milliseconds timeout{60 * 1000};
};
```

#### ErrorInfo
The ErrorInfo structure holds information about any errors that occur during the request.
```c++
struct ErrorInfo {
    /// The result code of the operation. Default is ResultCode::Success.
    ResultCode retCode = ResultCode::Success;

    /// The error code, system error code
    int32_t errorCode{};
};
```

#### ResponseHeader
The ResponseHeader structure holds the HTTP headers and status code received in the response.
```c++
struct ResponseHeader {
    /// A map of HTTP headers received in the response.
    std::unordered_map<std::string, std::string> headers;

    /// The HTTP status code of the response. Default is HttpStatusCode::Unknown.
    HttpStatusCode httpStatusCode = HttpStatusCode::Unknown;

    /// The reason phrase associated with the status code.
    std::string reasonPhrase;
};
```

#### ResponseHandler
The ResponseHandler structure manages various stages of the HTTP response lifecycle through callback functions.
```c++
struct ResponseHandler {
    /// Callback when the connection is established.
    using OnConnectedFunc = std::function<void(std::string_view)>;
    OnConnectedFunc onConnected = nullptr;

    /// Callback when the response headers are parsed.
    using ParseHeaderDoneFunc = std::function<void(std::string_view, ResponseHeader&&)>;
    ParseHeaderDoneFunc onParseHeaderDone = nullptr;

    /// Callback when data is received in the response.
    using ResponseDataFunc = std::function<void(std::string_view, DataPtr data)>;
    ResponseDataFunc onData = nullptr;

    /// Callback when the connection is closed.
    using OnDisconnectedFunc = std::function<void(std::string_view)>;
    OnDisconnectedFunc onDisconnected = nullptr;

    /// Callback when an error occurs.
    using OnErrorFunc = std::function<void(std::string_view, ErrorInfo)>;
    OnErrorFunc onError = nullptr;
};
```

#### Request Class
The Request class is used to initiate and manage HTTP requests.
```c++
class Request {
public:
    /// Initializes the request framework.
    static bool init();

    /// Clears the request framework.
    static void clear();

    /// Constructs a Request object with given RequestInfo and ResponseHandler. May cause performance degradation due to data copying.
    [[maybe_unused]] explicit Request(const RequestInfo&, const ResponseHandler& );

    /// Constructs a Request object with given RequestInfo and ResponseHandler using move semantics.
    [[maybe_unused]] explicit Request(RequestInfo&&, ResponseHandler&&);

    /// Destructor for the Request class.
    ~Request();

    /// Cancels the request.
    [[maybe_unused]] void cancel() noexcept {
        isValid_ = false;
    }

    /// Returns the request ID.
    [[maybe_unused]] [[nodiscard]] const std::string& getReqId() const {
        return reqId_;
    }
};
```
### Usage
##### 1.	Initialize the request framework:
```c++
Request::init(); //Must be callable on Windows.
```

##### 2. Create a RequestInfo object:
```c++
RequestInfo requestInfo;
requestInfo.url = "http://example.com";
requestInfo.methodType = HttpMethodType::GET;
```

##### 3.Create a ResponseHandler object:
```c++
ResponseHandler handler;
handler.onConnected = [](std::string_view reqId) { /* handle connection */ };
handler.onParseHeaderDone = [](std::string_view reqId, ResponseHeader&& header) { /* handle header parsing */ };
handler.onData = [](std::string_view reqId, DataPtr data) { /* handle data */ };
handler.onDisconnected = [](std::string_view reqId) { /* handle disconnection */ };
handler.onError = [](std::string_view reqId, ErrorInfo error) { /* handle error */ };
```

##### 4.Create a Request object:
```c++
Request request(requestInfo, handler);
```

##### 5.Cancel the request if needed:
```c++
request.cancel();
```

##### 6.Clear the request framework when done:
```c++
Request::clear(); //Must be callable on Windows.
```

##### 7. example
[example.cpp](example.cpp)

**If you prefer not to use HTTPS, set DISABLE_HTTPS to ON in the [CMake file](src/CMakeLists.txt).**
### Contributing
Feel free to contribute by opening issues or submitting pull requests. Please follow the code of conduct.

### License
This project is licensed under the MIT License. See the LICENSE file for details.
