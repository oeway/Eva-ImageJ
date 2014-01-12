Title: Machine learning plugin for Icy
Date: 2013-11-17 22:40
Tags: plugin, machine_learning
Slug: rapid-learning
Lang: en
Author: Will Ouyang
Summary:

Thanks to RapidMiner(formerly YALE) , which is a powerful and intuitive data mining tool implemented in Java and available under GPL (GNU General Public License).

The plugin, Rapid Learning, brings the power of RapidMiner to Icy, enables infinite probability, you can do all kinds things such as data loading and transformation (Extract, transform, load, a.k.a. ETL), data preprocessing and visualisation, modelling, evaluation, and deployment.

In Rapid Learning plugin, dimensions of (Z,T and C) are treated as features, packaged as an ExampleSet, the ExampleSet then can be used for training and predicting in RapidMiner process. 

Currently, you can do supervised learning and unsupervised learning to sequence. In supervised learning, you need to define some labels in Mask Editor and draw then on your sequence, then you can train your data and do predicting to other data. You also can do unsupervised learning like clustering, you will use Mask Editor to define your training set and then you can train the model and predicting other sequence.

Of course, you should download RapidMiner first and put the "lib" folder into "plugins" of Icy. Maybe I should make another plugin to include RapidMiner, but the package is too big to a icy library and I should find some way to enables the user use it as a standalone software.
You may also need the example process file needed by the plugin in the attachments. You can find some more information in the plugin page, about how to use them and how to generate it by yourself.

Since it's a preview edition, it works under my tests, there still many work to do,so suggestions are highly appreciated.
Welcome to have a try.

![RapidLearningPlugin][]

See more detail about the plugins, see [Rapid Learning](http://icy.bioimageanalysis.org/plugin/Rapid_Learning).

[RapidLearningPlugin]:http://i.imgur.com/kGPOzoR.png?1