# CDSV Reader

## Overview
`CDSVReader` is a class for reading DSV data from a specified data source (The data sink where our DSVWriter wrote to). Our DSVWriter was writing to a sink, and we are essentially reverse engineering this as our reader is reading from this sink and parses into row of strings.

## CDSVReader Class
```cpp 
DSVReader(std::shared_ptr< CDataSource > src, char delimiter);
~CDSVReader();
bool End() const;
bool ReadRow(std::vector<std::string> &row);
```

### `CDSVReader(std::shared_ptr< CDataSource > src, char delimiter);`

- This is the constructor that creates a DSV Reader, which takes in a pointer to a data source, and the delimiter character to separate values on.

### `~CDSVReader();`

- This is the destructor for the CDSVReader

### `bool End() const;`

- This returns true if the reader has reached the end of data source, or essentially when there is nothing left to read
- Returns false as long as we are able to peek next character (and that char is not empty string)

### `bool ReadRow(std::vector<std::string> &row);`

- This function returns true if it successfully read one row, and false otherwise
- It reads exactly one row from the data source and writes the parsed values into `row`
- Responsible (main logic) for reading rows from data sink
    - handles \n and edge cases for double quotes appearing in sink
    - splits column by delimiter
    - handles empty rows and fields --> treats as 1 row but with empty string

## Example Usage

### Writing
```cpp
std::shared_ptr<CStringDataSink> DataSink = std::make_shared<CStringDataSink>(); // allocate a new CStringDataSink object 
CDSVWriter Writer(DataSink,','); // pass data sink (write destination) into writer

Writer.WriteRow({"Project 2", "Is", "Awesome"});
Writer.WriteRow({"Cant", "Wait", "For", "Next", "One"});

std::cout << DataSink->String();

// will output
// Project2,Is,Awesome\nCant,Wait,For,Next,One
```
### Reading

```cpp
std::shared_ptr<CStringDataSource> DataSource = std::make_shared<CStringDataSource>();
CDSVReader Reader(DataSource, ',');

std::vector<std::string> Row1;
Reader.ReadRow(Row1); // reads into vector: ["Project 2", "Is", "Awesome"]

std::vector<std::string> Row2;
Reader.ReadRow(Row2); // reads into vector ["Cant", "Wait", "For", "Next", "One"]

std::vector<std::string> Row3;
Reader.ReadRow(Row3); // returns FALSE --> AT EOF
```






