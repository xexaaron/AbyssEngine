# Welcome to Abyss Engine Docs

## Choose a Section

<div class="grid cards" markdown>

-   :octicons-rocket-16:{ .lg .middle } [__Getting Started__](GettingStarted.md)
    
    ---
    
    Get started using Abyss Engine

{% for section in get_sections() %}
-   :{{ get_icon(section) }}:{ .lg .middle } [__{{ section | capitalize }}__](pages/{{ section }}.md/)

    ---

    Learn how to use the {{ section }} module.

{% endfor %}

</div>
