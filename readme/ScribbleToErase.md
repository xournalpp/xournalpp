# Scribble to erase

Scribble to erase removes whole ink strokes when you draw back and forth over
them with the Pen tool. To enable it, open **Preferences > Drawing Area** and
select **Enable scribble to erase**. The option is off by default.

Make several passes over the ink you want to remove, then release the mouse
button or lift the pen. The gesture can erase several strokes at once on the
selected visible layer. Undo restores the entire group in its original order;
Redo removes it again.

The feature works with both freehand drawing and shape recognition. Strokes
created by shape recognition can also be erased. Text, images, PDF backgrounds
and ink on other layers are preserved. The Highlighter and explicit shape tools
keep their usual behavior.

## Adjusting recognition

Expand **Advanced settings** below the checkbox:

- **Recognition sensitivity (0-100)** controls how closely a scribble must match
  the expected gesture. The default is **50**. Higher values accept less regular
  scribbles, but can also erase ordinary drawing by mistake.
- **Minimum passes** sets how many back-and-forth movements are needed. Each
  movement in one direction counts as one pass. The range is **2 to 8**, with a
  default of **4**. Lower values need less movement and increase the risk of
  unintended erasure. Moving across several targets may need extra passes.
- **Restore defaults** resets both values. Click OK to apply them or Cancel to
  discard the changes, including a reset. Applied values are saved across restarts.

Try one setting at a time on a copy of your document. Check ordinary writing and
hatching as well as the scribbles you want to use. Repeated narrow hatching can
resemble an erase gesture even at the default settings.

If a gesture is not recognized or does not repeatedly touch eligible ink, it is
handled as normal drawing or passed to shape recognition. Cancelling a gesture
does not erase anything. Settings apply to the next gesture; changing the tool,
drawing mode, zoom or recognition settings during a gesture prevents it from
erasing.

The controls follow the application's language setting. Translation improvements
can be submitted through [Crowdin](https://crowdin.com/project/xournalpp), as
described in [CONTRIBUTING.md](../CONTRIBUTING.md#contributing-translation-improvements).

## Implementation notes

`ScribbleGestureRecognizer` classifies the pen path using normalized page
coordinates, arc-length sampling and overlapping passes. A bounded fallback
handles dense, uneven scrubs that move across several targets. Recognition is
checked on release, before shape recognition.

`ScribbleEraser` checks contact with the actual ink, including line caps, dash
gaps, pressure widths and filled strokes. Bounding-box overlap alone does not
erase a stroke. All targets are resolved under the document lock before any
mutation. The existing `DeleteUndoAction` records the batch, and the input overlay
is cancelled instead of being committed to the page. History and input-context
checks reject gestures that span intervening edits or incompatible input changes.

Geometry work is limited to 32,768 expanded pieces per stroke, 2,000,000 segment
bounds checks and 250,000 weighted exact checks per gesture. Exceeding a limit
rejects the whole erase operation and keeps normal drawing behavior. These limits
bound the amount of work, not elapsed time. The feature adds no runtime dependency
or document-format change.

## Testing

Enable tests as described in [Compile.md](Compile.md#test), then build and run:

```sh
cmake --build build --target xournalpp test-units test-gtk-integration benchmarks translations
build/test/test-units
build/test/test-gtk-integration --gtest_filter='Scribble*' --gtest_repeat=3
ctest --test-dir build --output-on-failure --parallel 1
build/test/benchmarks --gtest_filter='ScribbleBenchmark.*' --gtest_repeat=3
```

GTK tests need a display. On headless Linux, run them with `xvfb-run -a`. Use an
isolated configuration for tests that create a controller or save preferences.
The tests cover recognized shapes, spaced targets, mouse and stylus dispatch,
both pen modes, settings save/cancel/reset, invalid input and atomic Undo/Redo.
Device events are synthetic and do not exercise physical device drivers.

`ScribbleTranslationTest` checks all nine messages in a real settings dialog using
a test-only catalog, independently of release translations from Crowdin. Set
`XOURNALPP_TEST_LANGUAGE` to a catalog name, such as `de` or `pt_BR`, or to
`en` for source English. The default catalog contains a Brazilian Portuguese test
translation. `XOURNALPP_TEST_LOCALE_DIR` selects an installed locale tree instead;
messages not yet translated there fall back to English. Run each language in a
separate process with a non-C UTF-8 locale available.
The feature fixtures finish asynchronous initialization before releasing their
test windows.

The benchmarks report release-handler timings for regular and recorded gestures,
with the feature enabled and disabled. They exclude repaint, window presentation
and physical-device latency. Set `XOURNALPP_SCRIBBLE_BENCH_SAMPLES` to a CSV path
to save individual observations; use a separate file for each benchmark.

The handwriting fixture is regression data, not an independent accuracy study.
Before release, test real mouse and pen input, ordinary writing and hatching,
cancellation, zoom, layers, Undo/Redo, and saving and reopening a document.
