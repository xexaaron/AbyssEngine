# Text

## Decorations

Rendered Text can be decorated using html like tags surrounding portions of the string.

- **Underline**: &lt;ul&gt;&lt;/ul&gt;
- **Higlight**: &lt;hl&gt;&lt;/hl&gt;

## Console Modifiers

Console logs can have modifiers using html like tags surrounding portions of the string. 

All decorations can be used in console text.

These tags tell our console to display items such as links properly and have them be clickable to navigate to the link.

- **Filepath**: &lt;fp&gt;&lt;/fp&gt;
- **URI/URL**: &lt;ur&gt;&lt;/ur&gt;

## Examples

!!! note "Note"
    Unlike HTML decorations support overlapping ranges.


=== "Decorations"

    ```cpp title="Highlight Followed By Underline"
    "<hl>highlighted portion</hl><ul>underlined portion</ul>"
    ```

    ```cpp title="Highlight With Nested Underline"
    "<hl><ul>HEADER</ul>INFO</hl>"
    ```

    ```cpp title="Highlight with overlapping Underline"
    "<hl>INFO<ul>HEADER</hl></ul>"
    ```

=== "Modifiers"

    ```cpp title="File Path"
    "<fp>DRIVE:/Path/To/File</fp>"
    ```

    ```cpp title="URL"
    "<ur>https://example.com</ur>"
    ```

    ```cpp title="FTP"
    "<ur>file:///Path/To/Local/File</ur>"
    ```
