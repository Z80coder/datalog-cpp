# Quick git how to

Quick instructions on how to contribute to this repo from the command line.

Please see link to other resources below.

## 1. Clone the repo on your local machine

```
git clone git@github.com:Z80coder/datalog-cpp.git
```

You should now have a `datalog-cpp` folder. 

```
cd datalog-cpp
```

## 2. Update the master branch


```
git checkout master
git pull
```

## 3. Create a new branch for your work

You will develop your code in your own branch. First you need to create it.

```
git checkout -b your_branch_name

git push --set-upstream origin your_branch_name
```

## 4. Or switch to your existing branch for your work

If you already done the step above, then ensure you are in your branch when you write code.

```
git checkout your_branch_name
```

## 5. Do some coding

Work as normal

## 6. Commit your changes

Type
```
git status
```
to check which files you've added or changed. Note the files you want to commit at this time, then:
```
git add newfile1 newfile2
```
Then commit the changes
```
git commit
```

## 7. Push your changes to GitHub

All you've done so far is declare some changes. You haven't yet pushed them to the GitHub repo. Let's do that now.

```
git push
```

You can keep adding, committing and pushing until you're ready to open a pull-request (with the intent of merging the changes in your branch into the `master` on the repo).

## 8. Open a pull-request

Go to https://github.com/Z80coder/datalog-cpp

Click on the `Branch` button and look for `your_branch_name`, and then click on it.

You should see a `New pull request` button. Click it and follow the user-interface prompts to create a pull-request.

Once reviewed, your branch will be merged. You can contribute a new collection of changes by returning to `Create a new branch for your work` above.

## Other resources

- Microsoft Visual Studio has git support built in, and there's a GitHub plugin for Visual Studio to enable direct cloning from GitHub repos.

- Microsoft Visual Code also has plug-in extensions that support Git

- https://guides.github.com/ has some useful guides.

- Also, there's a GitHub Desktop app: https://desktop.github.com/