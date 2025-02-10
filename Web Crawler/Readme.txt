 Files in the folder:
- Crawler.py - This is the main python file of the crawler
- logs.log - There are 2 log files each file contains logs of all the pages crawled by the crawler with other detials like time of crawl, size of pages, status code etc.
- Explain.txt - this file contain information about the percentage of languages, total pages traversed and total pages sampled along with the explaination of the code.
- samples folder - contains all the sampled pages from both crawls

Here's a list of the libraries used in the assignment, along with their uses:

1. requests - Used for making HTTP requests to web servers, enabling you to retrieve web content.
2. os - Provides functions for interacting with the operating system, allowing you to work with files, directories, and environment variables.
3. time - Offers various time-related functions, such as measuring time intervals, delaying execution, and working with timestamps.
4. logging - Used for creating log files and messages to help with debugging and monitoring your Python programs.
5. urllib.parse - Provides functions for parsing and manipulating URLs, making it easier to work with web addresses.
6. urljoin - Specifically used for joining parts of URLs together to create valid URLs.
7. bs4 (Beautiful Soup) - A library for parsing HTML and XML documents, primarily used for web scraping and data extraction from web pages.
8. urllib.robotparser - Allows you to parse and interpret robots.txt files from websites to understand crawling permissions for web crawlers.
9. datetime - Provides classes and functions for working with dates and times, making it useful for timestamp manipulation.
10. random - Used for generating random numbers, which can be useful in various scenarios, including simulations and randomization.
11. queue - Provides a queue data structure, which is useful for implementing various algorithms and solving problems that involve managing items in a first-in-first-out (FIFO) order.
12. collections.defaultdict - A specialized dictionary data structure that provides default values for keys that do not exist, useful for counting and aggregating data.
13. langdetect - Used for language detection in text data, allowing you to identify the language of a given text.

- To compile and run the crawler, first we need to download required libraries using pip install.
- Then, we need to compile the crawler.py file and run it in the termianl or python enviornment.

