if [ $# -eq 1 ]; then
	echo "Making a Neologism release package...\n"

	# Download Drupal Core, at the moment we can not use "drush dl drupal" because drush does not support download drupal version
	drush dl drupal
	mv drupal-6.* neologism
	cd neologism

	# Download required modules
	echo "Downloading required drupal modules by Neologism...\n"
	drush dl cck rdf ext rules

	# Download and extract ARC, which is required as part of the RDF module
	echo "Downloading second part required packages...\n"
	mkdir sites/all/modules/rdf/vendor
	curl -o sites/all/modules/rdf/vendor/arc.tar.gz http://code.semsol.org/source/arc.tar.gz
	tar xzf sites/all/modules/rdf/vendor/arc.tar.gz -C sites/all/modules/rdf/vendor/
	rm sites/all/modules/rdf/vendor/arc.tar.gz

	# Download and extract ExtJS-3, which is required for the evoc module 
	curl -O http://extjs.cachefly.net/ext-3.0.0.zip 
	unzip ext-3.0.0.zip
	mv ext-3.0.0 sites/all/modules/ext/
	rm ext-3.0.0.zip

	echo "Checking out main Neologism modules...\n"
	# Check out Neologism and evoc modules from Google Code SVN
	# @@@ use export instead???
	svn co https://neologism.googlecode.com/svn/branches/DRUPAL-6--14/neologism sites/all/modules/neologism --username $1
	svn co https://neologism.googlecode.com/svn/branches/DRUPAL-6--14/evoc sites/all/modules/evoc --username $1

	# Check out Neologism installation profile from Google Code SVN
	svn co https://neologism.googlecode.com/svn/branches/DRUPAL-6--14/profile profiles/neologism --username $1 

	# Delete the Drupal default installation profile, we only support the Neologism one
	rm -rf profiles/default/

	# Create archive of the entire thing, ready for installation by users
	cd ..
	zip -r neologism.zip neologism
else
	echo "Usage: make-archive [username]\n"
fi
