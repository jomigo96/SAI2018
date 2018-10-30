# fetcher.py

Script to extract route table from [flightplandatabase](https://flightplandatabase.com) and write a file with the format expected from our program.

## Usage

Run the script, enter the URL and destination file as prompted.

Example:
```
python3 fetcher.py
URL: https://flightplandatabase.com/plan/1362792
Destination file name: my_file.txt
```

Writes the following content:
```
NONE;64°11'27.240000"N 51°40'41.160000"W 0ft;
ASVID;64°37'49.080000"N 48°22'44.040000"W 15000ft;
SOBVI;65°10'54.840000"N 43°49'44.040000"W 15000ft;
ASTAN;65°25'4.080000"N 39°59'44.160000"W 15000ft;
DA;65°34'14.160000"N 37°12'24.840000"W 15000ft;
NASOP;65°34'0.120000"N 34°34'59.880000"W 15000ft;
NONRO;65°10'59.880000"N 30°00'0.000000"W 15000ft;
NONE;63°58'53.760000"N 22°36'52.200000"W 0ft;
```

