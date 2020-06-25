
# FEP Timing Interface {#page_fep_timing}

## FEP timing for 2.3 and higher

See @ref page_fep_timing_3


## FEP timing for FEP 2

See @subpage fep_timing_2

FEP 2.3 comes with a FEP Timing Legacy Layer. This layer is separated into two parts:

1. FEP 2 Legacy API (your code will compile until the end of the FEP 2 life cycle)
2. FEP 2 Functionality Legacy (you may connect to "old" participants and use FEP 2 timing within your
FEP system)

The compatibility between the FEP 2.2.0 Timing and FEP 2.3.0 <b>Timing Legacy</b> layer is guaranteed. 
In order to be compatible the backlog size of the matching signals needs to be equal. It is recommended 
to use a timing configuration file (see @ref fep_timing_2) to configure the appropriate property.

\note When using the *FEP 2 Legacy API* the @ref fep::InputViolationStrategy (see @ref fep::InputConfig) with *FEP 3 Timing* configured for a job/task is not applied.
