## OTTER Tools for TAs

This repo contains the source code for tools provided for TAs to ease marking of projects created in OTTER

Currently there are 2 main tools:

#### OTTER_Project_Pack
This project creates an EXE that is designed to sit next to the OTTER premake files. Running this program will provide students wil a console interface to pack an OTTER
project into a zip file for submission. The file naming and layout are pre-determined, which allows us to use tools for extracting the zips in bulk

#### OTTER_Project_Unpack
This project creates an EXE that TAs can use to download and unpack project submissions in bulk. As long as students have used the *CreateSubmission* tool mentioned above, 
the unpack tool should be capable of auto-generating projects from it.

When using the Unpack tool, you will be prompted for a zip file containing the submissions, and the Premake.lua file of an OTTER repo to extract the projects into. The name
of the zipfile will be used to determine a folder name under the `samples` directory. All projects will be extracted from the provided zip file, and a basic validation will
be run against them. It will check all submissions for a `res` and `src` folder, and display an error if the project does not contain these essential folders. A message will
also display at the end of the run indicating the total number of submissions, as well as the number of invalid submissions.