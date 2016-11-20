#!/usr/bin/python

import sys
from temboo.Library.Facebook.Publishing import Post
from temboo.core.session import TembooSession


statusMsg = sys.argv[1]
linkMsg = sys.argv[2]

# Create a session with your Temboo account details
session = TembooSession("accountName", "myFirstApp", "abc123xxxxxxxxxxxxxx")

# Instantiate the Choreo
postChoreo = Post(session)

# Get an InputSet object for the Choreo
postChoreInputs = postChoreo.new_input_set()

# Set the Choreo inputs

postChoreInputs.set_Link(linkMsg)
postChoreInputs.set_Message(statusMsg)
postChoreInputs.set_AccessToken("COLOQUE SEU TOKEN DO FACEBOOK AQUI")

# Execute the Choreo
print("Msg: " + statusMsg)
print("Link: " + linkMsg)
postChoreResults = postChoreo.execute_with_results(postChoreInputs)

# Print the Choreo outputs
print("Response: " + postChoreResults.get_Response())
