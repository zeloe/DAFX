juce_add_binary_data(BinaryData SOURCES
                    files/IR.aif


)
# Make linux happy
set_target_properties(BinaryData PROPERTIES POSITION_INDEPENDENT_CODE TRUE)