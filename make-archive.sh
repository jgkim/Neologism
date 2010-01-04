# Download Drupal Core
drush dl drupal

# Rename directory created by above command
mv drupal-6.14 neologism-1.0-rc4
cd neologism-1.0-rc4

# Download required modules
drush dl cck rdf sparql ext rules

# Download and extract ARC, which is required as part of the RDF module
mkdir sites/all/modules/rdf/vendor
curl -o sites/all/modules/rdf/vendor/arc.tar.gz http://code.semsol.org/source/arc.tar.gz
tar xzf sites/all/modules/rdf/vendor/arc.tar.gz -C sites/all/modules/rdf/vendor/
rm sites/all/modules/rdf/vendor/arc.tar.gz

# Download and extract ExtJS-3, which is required for the evoc module 
wget http://www.extjs.com/products/extjs/download.php?dl=extjs3
tar xzf ext-3.0.0.zip
mv ext-3.0.0 sites/all/modules/ext/
rm ext-3.0.0.zip

# Check out Neologism and evoc modules from Google Code SVN
# @@@ use export instead???
svn co https://neologism.googlecode.com/svn/trunk/neologism sites/all/modules/neologism --username richard@cyganiak.de
svn co https://neologism.googlecode.com/svn/trunk/evoc sites/all/modules/evoc --username richard@cyganiak.de

# Check out Neologism installation profile from Google Code SVN
svn co https://neologism.googlecode.com/svn/trunk/profile profiles/neologism --username richard@cyganiak.de

# Delete the Drupal default installation profile, we only support the Neologism one
rm -rf profiles/default/

# Create archive of the entire thing, ready for installation by users
cd ..
zip -r neologism-1.0-rc4.zip neologism-1.0-rc4
