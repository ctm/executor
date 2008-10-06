executor-faq.ascii: executor-faq.bfnn
	(cd ../packages/linuxfaq ; make executor-faq.ascii)

gestalt/t1.html: gestalt.xml
	docbook2html gestalt.xml -o gestalt
	rsync -e ssh -avzu gestalt buy:/var/www/html/new/

faq/t1.html:	faq.xml
	docbook2html faq.xml -o faq
	rsync -e ssh -avzu faq buy:/var/www/html/new/
