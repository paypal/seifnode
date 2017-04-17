# Contributing To seifnode

We are always looking for ways to make our modules better. Adding features and fixing bugs allows everyone who depends
on this code to create better, more stable applications.
Feel free to raise a pull request to us. Our team would review your proposed modifications and, if appropriate, merge
your changes into our code. Ideas and other comments are also welcome.

## Getting Started
1. Create your own [fork](https://help.github.com/articles/fork-a-repo) of this [repository](../../fork).

```bash
# Clone it
git clone git@github.com:me/seifnode.git

# Change directory
cd seifnode

# Add the upstream repo
git remote add upstream git://github.com/paypal/seifnode.git

# Get the latest upstream changes
git pull upstream

# Configure project and install dependencies
npm install

# Run tests
npm test
```

## Making Changes
1. Make sure that your changes adhere to the current coding conventions used throughout the project, indentation, accurate comments, etc.
2. Ensure that all the tests pass.

## Submitting Changes
1. Commit your changes in logical chunks, i.e. keep your changes small per single commit.
2. Locally merge (or rebase) the upstream branch into your topic branch: `$ git pull upstream && git merge`.
3. Push your topic branch up to your fork: `$ git push origin <topic-branch-name>`.
4. Open a [Pull Request](https://help.github.com/articles/using-pull-requests) with a clear title and description.

If you have any questions about contributing, please feel free to contact us by posting your questions on GitHub.

Copyright 2015, 2016, 2017 PayPal under [The MIT License](LICENSE.md).
