.. _hello_world:

Hello, Cyclus!
==============
This pages walks you through a very simple hello world example using
|cyclus| agents.  First make sure that you have the dependencies installed,
namely |Cyclus|, CMake, and a recent version of Python (2.7 or 3.3+).

First, you need to get the ``cycstub`` code.  Cycstub is a skeleton code base
that you can use to quick-start new |cyclus| module development projects.
You can grab cycstub from |cyclus|.

Let's put this code in a ``tutorial`` directory and go into it.

**Getting cycstub via Cyclus:**

.. code-block:: bash

    $ mkdir tutorial
    $ cd tutorial/
    $ cycstub --type facility .../flexmore:flexmore:tutorialfacility

------------

Let's now change the behavior of the TutorialFacility's ``Tick()`` &
``Tock()`` member functions to print "Hello" and "World" respectively.  To do
this, please open up the :file:`src/tutorial_facility.cc` file in your
favorite text editor (vim, emacs, gedit, `notepad++ <http://exofrills.org>`_).
Change the original functions to look like:

**Original Tick() and Tock() in src/tutorial_facility.cc:**

.. code-block:: c++

    void TutorialFacility::Tick() {}

    void TutorialFacility::Tock() {}

**New Tick() and Tock() in src/tutorial_facility.cc:**

.. code-block:: c++

    void TutorialFacility::Tick() {std::cout << "Hello, ";}

    void TutorialFacility::Tock() {std::cout << "World!\n";}

------------

Now that we have altered the behavior of the TutorialFacility, let's compile and
install the ``tutorial`` project.  This done with the install.py script.
The install script puts the project into your cyclus userspace,
``${HOME}/.local/lib/cyclus``.

.. code-block:: bash

    tutorial $ python install.py

