# MPI_T Events Tests

If the MPI implementation includes support for the MPI_T Events functions as defined in [MPI Standard 4.0](https://www.mpi-forum.org/docs/mpi-4.0/mpi40-report.pdf) and includes some implemented events, these tests will confirm basic functionality as well as some error checking.

## Events Callback Test Requirements
In order to confirm the MPI_T Events callback functionality, a callback must be registered and triggered by performing relevant MPI activity.

The events_types test will print the list of events available by providing the command line '-l' flag.

The events_callbacks event index can be set for the events_callbacks test with the `-i [number]` flag.

If the default MPI activity in the function generate\_callback\_activity() in events_common.c does not trigger the event callback, custom MPI activity will need to be added.

## Test Behavior

The behavior of each test can be modified with the following command line arguments:

- -d : print internal debugging messages [default: off]
- -e : print error messages [default: off]
- -f : perform failure tests [default: off]
- -i [number] : event index for specific tests [default: 0]
- -l : list available events (events_types only) [default: off]

## Test Descriptions

- events_callbacks
  - Register and free a callback function for an event
  - Get and set event handle and callback info
- events_dropped
  - Register a callback function for dropped events
- events\_meta_data
  - Get event timestamp and source
- events\_read_data
  - Read and copy event data
- events_source
  - Get number of source, source info object, and source timestamp
- events_types
  - Get number of events and event info object

## Event Callback and Read Data Example
The events_example.c file has been provided as an example of registering an event callback and reading event data.
It can be used with the Open MPI '--mca pml ob1' mpiexec flags to generate MPI_T Event callback behavior and
confirm callback functionality.

## Known Open MPI MPI_T Event test failures

- MPI_T_event_handle_free : user_data is not accessible in callback function
- MPI_T_event_handle_set_info, MPI_T_event_callback_set_info : keys are not added to Info objects

## Possible Additional Tests

- MPI_T_event_set_dropped_handler : Functionality is not currently implemented for dropped events in Open MPI, but a test to co nfirm that the dropped handler is called could be useful.

## Test Suite And MPI Implementations

Tested with [Open MPI PR #8057](https://github.com/open-mpi/ompi/pull/8057) with pml ob1 module events.
