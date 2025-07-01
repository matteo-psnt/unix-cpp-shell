# Integration tests using Tcl/Expect

# Find the expect interpreter
find_program(EXPECT_EXEC expect REQUIRED)

if(NOT EXPECT_EXEC)
    message(FATAL_ERROR "expect not found! Please install expect: brew install expect (macOS) or sudo apt-get install expect (Ubuntu)")
endif()

# Create a custom target for all integration tests
add_custom_target(shell_integration_tests
    COMMENT "Running shell integration tests"
    DEPENDS shell
)

# Helper function to add integration tests
function(add_integration_test test_name script_name)
    add_test(NAME integration_${test_name}
        COMMAND ${EXPECT_EXEC} ${CMAKE_CURRENT_SOURCE_DIR}/${script_name}.exp
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    
    # Set test properties
    set_tests_properties(integration_${test_name} PROPERTIES
        LABELS "integration"
        TIMEOUT 30
    )
    
    # Make sure shell is built before running integration tests
    add_dependencies(shell_integration_tests shell)
endfunction()

# Register integration test scripts
add_integration_test(simple_command simple_command)
add_integration_test(pipeline_redirection pipeline_redirection)
add_integration_test(tab_completion tab_completion)
add_integration_test(aliases aliases)
add_integration_test(advanced_features advanced_features)
