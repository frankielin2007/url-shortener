## Technologies

- C++20
- Crow
- SQLite
- CMake
- Ninja
- MSYS2 UCRT64

## How to Run

### Requirements

The project was developed using:

- Windows
- MSYS2 UCRT64
- GCC/G++
- CMake
- Ninja

The Crow and SQLite dependencies are configured for the MSYS2 UCRT64 environment.

### 1. Open MSYS2 UCRT64

### 2. Navigate to the project

### 3. Configure the project

```bash
cmake -S . -B build -G Ninja
```

### 4. Build the project

```bash
cmake --build build
```

### 5. Start the server

```bash
./build/url_shortener.exe
```

The server will run at:

```text
http://localhost:8080
```

The response should be:

```text
Url shortener service is running
```

## API Usage

### POST `/shorten`

Creates a shortened URL.

#### Request

```json
{
    "url": "https://www.instagram.com"
}
```

#### Example using PowerShell

```powershell
$body = '{"url":"https://www.instagram.com"}'

$result = Invoke-RestMethod `
    -Uri "http://localhost:8080/shorten" `
    -Method Post `
    -ContentType "application/json" `
    -Body $body

$result
```

#### Example response

```text
short_url                    original_url               code
----------                   ------------               ----
http://localhost:8080/Ab12Cd https://www.instagram.com  Ab12Cd
```

The API returns `201 Created` when the URL is successfully shortened.

---

### GET `/:code`

Redirects the shortened URL to the original URL.

For example, if the generated code is `Ab12Cd`, open:

```text
http://localhost:8080/Ab12Cd
```

The server returns a `302 Found` redirect to the original URL.

Each successful redirect increases the access count by one.

#### Example using PowerShell

```powershell
Invoke-WebRequest `
    -Uri "http://localhost:8080/Ab12Cd" `
    -MaximumRedirection 0 `
    -UseBasicParsing
```

The response should contain:

```text
StatusCode : 302
Location   : https://www.instagram.com
```

---

### GET `/:code/stats`

Returns information about a shortened URL and its access count.

#### Example using PowerShell

```powershell
Invoke-RestMethod `
    -Uri "http://localhost:8080/Ab12Cd/stats"
```

#### Example response

```json
{
    "code": "Ab12Cd",
    "original_url": "https://www.instagram.com",
    "access_count": 1
}
```

## AI Usage

AI tools and publicly available documentation were used during development.

The implementation was tested locally, and the code and design decisions can be explained during the interview.