import requests
import os
import time
import logging
from urllib.parse import urlparse, urljoin
from bs4 import BeautifulSoup
from urllib.robotparser import RobotFileParser
from datetime import datetime
import random
import queue
from urllib.parse import urlparse, urljoin
import logging
from datetime import datetime
from collections import defaultdict
from langdetect import detect
import ssl

ctx = ssl.create_default_context()
ctx.check_hostname = False
ctx.verify_mode = ssl.CERT_NONE

# Custom Search Engine ID and API key
cse_id = "84f149c6133bf4d0c"
api_key = "AIzaSyCezfuCYq0odDWMQZJT7mKqgy8MmaH8HnI"
# Define the keyword or topic for search
query = "Wikipedia"

num_results = 10
# URL for the Google Custom Search API request
url = f"https://www.googleapis.com/customsearch/v1?key={api_key}&cx={cse_id}&q={query}&num={num_results}"
# Store the URLs in a list
seed_urls = []

domain_frequency = defaultdict(int)
visited_urls = set()
sampled_urls = set()
sample_threshold = 0.1
error_404_count = 0
max_domain_appearances = 100

language_counts = {
    "English": 0,
    "Spanish": 0,
    "Chinese": 0,
    "Dutch": 0,
    "Other": 0,
    "Error": 0
}

sampled_pages = 0  # Initialize the counter for sampled pages
domain_links = defaultdict(list)


#lo gger
logger = logging.getLogger('my_logger')
logger.setLevel(logging.DEBUG)

# log filename with a timestamp
log_filename = f"crawler_logs_{time.strftime('%Y%m%d%H%M%S')}.log"

# file handler and its level
handler = logging.FileHandler(log_filename)
handler.setLevel(logging.DEBUG)

# formatter attached to the handler
formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
handler.setFormatter(formatter)

# Adding the handler to the logger
logger.addHandler(handler)

try:
    # Send the API request
    response = requests.get(url, verify=False, timeout=5)

    # Check if the request was successful
    if response.status_code == 200:
        # Parse the JSON response
        data = response.json()

        # Extract and print the seed URLs
        for item in data.get("items", []):
            url = item.get("link")
            print(url)
            if url not in seed_urls:
                seed_urls.append(url)
    else:
        logger.error(f"Error: Unable to fetch seed URLs. Status Code: {response.status_code}")
except requests.exceptions.RequestException as e:
    logger.error(f"Error: Request Exception - {e}")
except ValueError as ve:
    logger.error(f"Error: JSON Parsing Error - {ve}")
except Exception as ex:
    logger.error(f"Error: An unexpected error occurred - {ex}")
    

def setup_robot_parser(base_url):
    robot_parser = RobotFileParser()
    robot_txt_url = urljoin(base_url, "/robots.txt")
    
    try:
        robot_parser.set_url(robot_txt_url)
        robot_parser.read(ctx=ctx)
    except Exception as e:
        logger.error(f"Error while fetching or parsing robots.txt from {robot_txt_url}: {e}")
    return robot_parser

def should_sample():
    
    return random.random() < sample_threshold

def log_page_info(url, status_code, page_size, pg_lang, s):
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    if s == 1:
        logger.info(f"|URL: {url}|Status Code: {status_code}|Sampled: Yes|{pg_lang}|Page Size: {page_size} bytes")
    elif s == 0:
        logger.info(f"|URL: {url}|Status Code: {status_code}|Sampled: No|{pg_lang}|Page Size: {page_size} bytes")

def download_page(url):
    try:
        response = requests.get(url)
        if response.status_code == 200:
            # The timestamp 
            timestamp = datetime.now().strftime("%Y%m%d%H%M%S")

            # Creates the "sample" folder if it doesn't exist
            os.makedirs("sample", exist_ok=True)

            # Generates a file name based on the timestamp
            file_name = f"{timestamp}.html"

            # Writes the downloaded content to the file in the "sample" folder
            with open(os.path.join("sample", file_name), "wb") as file:
                file.write(response.content)

            print(f"Page downloaded and saved to sample/{file_name}")
        else:
            logger.error(f"Failed to download the page. Status code: {response.status_code}")
    except Exception as e:
        logger.error(f"An error occurred: {str(e)}")

def detect_page_language(soup):
    global language_counts, sampled_pages
    try:
        text = soup.get_text()
        language = detect(text)

        # Map specific languages to their names
        language_names = {
            "en": "English",
            "es": "Spanish",
            "zh-cn": "Chinese",
            "nl": "Dutch"
        }

        # If the language is in the specific languages, update the counter
        detected_language = "Unknown"
        if language in language_names:
            detected_language = language_names[language]
            language_counts[detected_language] += 1
        else:
            language_counts["Other"] += 1  # Increment "Other" counter for unknown languages

        # Increment the sampled pages counter
        sampled_pages += 1
        return detected_language
    except Exception as e:
        logger.error(f"Error while detecting language: {str(e)}")
        language_counts["Error"] += 1  # Increment "Error" counter in case of errors
        return "Error"

def crawl_web(seed_urls, max_count, crawl_delay):

    global error_404_count
    internal_links_visited = 0
    url_queue = []
    url_queue_size_counter = 0
    traversed_counter =0
    sampled_counter =0
    
    
    
    for seed_url in seed_urls:
        url_queue.append(seed_url)
        url_queue_size_counter += 1
    
    while url_queue and len(sampled_urls) < max_count:
        current_url = url_queue.pop()
        url_queue_size_counter -= 1
        
        base_url = urlparse(current_url).netloc
        robot_parser = setup_robot_parser(current_url)
        
        if robot_parser is None:
            continue

        if current_url in visited_urls:
            if response.status_code == 404:  # Check for a 404 error
                error_404_count += 1 
            continue

        visited_urls.add(current_url)
        traversed_counter += 1
        print(f"CRAWLING: {current_url} {traversed_counter}")

        try:
            response = requests.get(current_url, timeout=5)
            if response.status_code != 200:
                continue

            content_type = response.headers.get("Content-Type", "")
            if "text/html" not in content_type or "application/pdf" in content_type:
                continue

            soup = BeautifulSoup(response.text, "html.parser")
            pg_lang = detect_page_language(soup)
            if should_sample() and len(soup.get_text(strip=True)) > 0 and current_url not in sampled_urls:
                sampled_urls.add(current_url)
                sampled_counter += 1
                download_page(current_url)
                print(f"*************SAMPLED: {current_url} {sampled_counter} {pg_lang}")
                log_page_info(current_url,{response.status_code}, len(response.content), pg_lang, 1)
            else:
                log_page_info(current_url, {response.status_code}, len(response.content), pg_lang, 0)

            for link in soup.find_all("a"):
                href = link.get("href")
                if href:
                    next_url = urljoin(current_url, href)
                    domain = urlparse(next_url).netloc
                    
                    if domain_frequency[domain] <= max_domain_appearances:
                        if robot_parser.can_fetch("*", next_url):
                            time.sleep(crawl_delay)
                            domain_links[domain].append(next_url)
                            domain_frequency[domain] += 1
                    else:
                    # Remove the domain from the dictionary
                        if domain in domain_links:
                            del domain_links[domain]
                        
            # for domain in domain_links:
            #     random.shuffle(domain_links[domain])
                
            mixed_links = []
            for links in domain_links.values():
                mixed_links.extend(links)

            extract_counter = min(200, len(mixed_links))   # Take up to 250 links
            mixed_links = random.sample(mixed_links, k=extract_counter)
            url_queue_size_counter += extract_counter

            # Push the mixed links into the url_queue
            url_queue = mixed_links + url_queue

            if url_queue_size_counter > 4500:
                url_queue = random.sample(url_queue, k=3000)
                url_queue_size_counter = 3000
            
            print("url_queue_size_counter",url_queue_size_counter)

        except (requests.exceptions.RequestException, Exception) as e:
            if isinstance(e, requests.exceptions.Timeout):
                logger.error(f"Timeout while crawling {current_url}: {str(e)}")  # Handle timeout exception
            else:
                logger.error(f"Error while crawling {response.status_code} {current_url}: {str(e)}")
            
            
    print(f"Traversed URLs: {len(visited_urls)}")
    print(f"Sampled URLs: {len(sampled_urls)}")

if __name__ == "__main__":
    seed_urls = seed_urls 
    max_count = 1000  # Maximum number of pages to crawl per seed URL
    crawl_delay = 0  # Delay between requests (in seconds)

    crawl_web(seed_urls, max_count, crawl_delay)
    
    # Calculate percentages
    language_percentages = {lang: (count / sampled_pages) * 100 for lang, count in language_counts.items()}
    print("language sample space: ",sampled_pages)
    # Print language percentages at the end of the code run
    for language, percentage in language_percentages.items():
        print(f"{language}: {percentage:.3f}%")



