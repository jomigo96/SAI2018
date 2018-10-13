import urllib.request
import math
from html.parser import HTMLParser
from lxml import etree

class TableParser(HTMLParser):
	
	def __init__(self):
		self.start = []
		self.end = []
		self.flag = False
		HTMLParser.__init__(self)
	
	def handle_starttag(self, tag, attrs):
		"""Finds the target table"""
		
#		if tag == 'table':
#			print('Table with attributes: {}'.format(attrs))
		
		
		if tag == 'table' and 'plan-route-table' in attrs[0]:
			self.flag = True
			#print("Found it")
			self.start = self.getpos()
			
			
	def handle_endtag(self, tag):
		"""Stops caching after table ends"""
		if tag == 'table' and self.flag:
			self.end = self.getpos()
			self.flag = False


if __name__ == "__main__":

	url = input("URL: ")
	file_name = input("Destination file name: ")

	#url = 'https://flightplandatabase.com/plan/1299629'
	#file_name="new.txt"

	parser = TableParser()


	fp = urllib.request.urlopen(url)
	bytestream = fp.read()
	fp.close()
	strdata = bytestream.decode("utf8")

	parser.feed(strdata)
	parser.close()


	strdata = strdata.splitlines()
	strdata = strdata[parser.start[0]-1:parser.end[0]]
	table = etree.HTML("".join(strdata)).find("body/table")
	rows = iter(table)
	next(rows)
	
	h=[]
	coors=[]
	idd=[]
	
	for row in rows:
		values = [col.text for col in row]
		coors.append(values[5])
		h.append(values[4])
		idd.append(values[1])
	
	idd = [a if a != None else 'NONE' for a in idd ]
	
	coors=[c.split('/') for c in coors]
	coors=[[c[0].replace(' ',''), c[1].replace(' ','')] for c in coors]
	h=[c.split('/')[0] for c in h]
	h=[c.replace(',', '').replace(' ','') for c in h]
	
	with open(file_name, 'w') as fp:
		for i in range(len(h)):
			lat = float(coors[i][0])
			lon = float(coors[i][1])
			char_lat = 'N' if lat >= 0 else 'S'
			char_lon = 'E' if lon >= 0 else 'W'
			lat = abs(lat)
			lon = abs(lon)
			lat_deg = math.floor(lat)
			lon_deg = math.floor(lon)
			lat_min = math.floor((lat-lat_deg)*60)
			lon_min = math.floor((lon-lon_deg)*60)
			lat_sec = (lat - lat_deg - lat_min/60) *3600
			lon_sec= (lon - lon_deg - lon_min/60) *3600
			fp.write('{0};{1:.0f}°{2:02.0f}\'{3:.6f}\"{4} {5:.0f}°{6:02.0f}\'{7:.6f}\"{8} {9}ft;\n'.format(
				idd[i], math.floor(lat), lat_min, lat_sec, char_lat, 
				math.floor(lon), lon_min, lon_sec, char_lon, h[i]))
	print("Finished")
