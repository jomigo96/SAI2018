import urllib.request
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
	
	fp=open(file_name, 'w')
	
	for i in range(len(h)):
		fp.write("{};{};{};{}\n".format(idd[i], coors[i][0], coors[i][1], h[i]))
		
	fp.close()
	
	print("Finished")
	

