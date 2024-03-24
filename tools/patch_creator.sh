make -C .. clean
git remote add upstream https://github.com/snipersim/snipersim.git
git fetch upstream
git merge upstream/main
git diff upstream/main > ../sniper_to_donuts.patch