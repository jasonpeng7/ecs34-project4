# CDSV Writer

## Overview
`CDSVWriter` is a class for writing DSV data to a specified data sink or destination. DSV specifically is a way of storing 'tabular data' by separating the fields of each row with a character or the delimiter. The most common forms of DSV data are CSV (Comma Separated Values) and TSV (Tab Separated Values) and these are files.

## CDSVWriter Class
```cpp 
CDSVWriter(std::shared_ptr< CDataSink > sink, char delimiter, bool quoteall = false);
~CDSVWriter();

bool WriteRow(const std::vector<std::string> &row);
```

### `CDSVWriter(std::shared_ptr< CDataSink > sink, char delimiter, bool quoteall = false);`

- This is the constructor that creates a DSV Writer, which takes in a pointer to a data sink, the delimiter character, and a `quoteall` flag, what is false by default
    - The `quoteall` flag basically wraps the value (column) in quotes, even if not required when `true`

### `~CDSVWriter();`

- This is the destructor for the CDSVWriter

### `bool WriteRow(const std::vector<std::string> &row);`

- This returns true if a row is successfully written, one string per column should be put in the row vector
- This is the core function that is actually handling the logic to write to a datasink and handle all edge cases.

## Example Usage

```cpp
std::shared_ptr<CStringDataSink> DataSink = std::make_shared<CStringDataSink>(); // allocate a new CStringDataSink object 
CDSVWriter Writer(DataSink,','); // pass data sink (write destination) into writer

Writer.WriteRow({"Project 2", "Is", "Awesome"});
Writer.WriteRow({"Cant", "Wait", "For", "Next", "One"});

std::cout << DataSink->String();

// will output
// Project2,Is,Awesome\nCant,Wait,For,Next,One
```









